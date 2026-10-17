// cc13_sweepB9: ninth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB9, docs/CC13_SWEEPB9.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (rule 4c): these functions run on the main thread, whose x87
// precision control is 24 bits (docs/CC13_FPU_PRECISION.md), so every x87
// arithmetic operation is modelled as rounding to float in the image's order.
// SSE arithmetic is float as written. EH frames are noted, not modelled.

#include "bsp/cc13_sweepB9.hpp"

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
void release_string(Cc13SweepBNativeString& text, Cc13SweepB9Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined temporary native string of a literal: {0, 0}, 0041DD40(length,
// 1), memcpy of length + 1 bytes when data is non-null, the use, then the
// release (the length re-read after the use).
template <class Use>
void with_c_string(const char* source, std::uint32_t length, Cc13SweepB9Host& host, Use use) {
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
    use(&text);
    release_string(text, host);
}

// 00E198C4, the in-game GUI.
void* in_game_gui(Cc13SweepB9Host& host) {
    return host.global_00e198c4();
}

}  // namespace

// ---------------------------------------------------------------------------
// 006179D0 BSP_HudPowerupCarousel_RebuildSlots
// __fastcall(ECX = carousel), RET. EH frame 00C77CD0 (states 0..19h track the
// name temporaries). Image facts (asm 006179D0..0061897A); sp below is
// [[00E198C4]+C0h], re-read at every access as the image does, bc is
// [[00E198C4]+BCh]:
// - +3Fh = 1; bc+100h and bc+104h vcall +34h(0).
// - +3Ch == 0 (00617A3F not taken, "up"): sp+21Ch = sp+28h, sp+220h = sp+48h;
//   for off = 38h, 3Ch, 40h (n = 0, 1, 2; counter [ESP+10h] starts at 1 and
//   0043C130's number is counter - 1): sp+off-10h = sp+off-0Ch, sp+off+40h =
//   sp+off+44h, sp+off+10h = sp+off+14h. Otherwise ("down", 006181DF):
//   sp+21Ch = sp+34h, sp+220h = sp+54h; for off = 44h, 40h, 3Ch (n = 3, 2,
//   1, the counter itself): the same three stores with the sources at
//   off-14h, off+3Ch and off+0Ch.
// - Each pass then builds "circle_full_" (00CF4C4C) + 004260B0(n) + "_Section"
//   (00CF4C5C) with 0043C130 / 004261A0 (temporaries released prefixed,
//   number, suffix), hides sp+off (vcall +34h(0)), stores 00AA7E00(sp+off-10h,
//   name, 1) to sp+off, shows it and vcall +4Ch(0.5f, 00CE3800), then
//   releases the name.
// - The new slot (clone C = 34h up / 28h down, icon I = 54h / 48h, highlight
//   H = 84h / 78h): 00AAB4C0(bc+138h, 0) to sp+C, shown; "icon_L_Icon"
//   (00CF4DDC) under sp+C to sp+I; "number_Text" (00CF4DD0) under sp+I
//   hidden; sp+I vcall +88h(3, 0, 1.0f); for i from (short) sp+I vcall +98h()
//   while i < sp+1C8h + start (unsigned JC, bound re-read each pass): the map
//   sp+1C0h is subscripted with key i twice (006170F0; the first result's
//   value gives +D4h, the second's +D0h) and 00AB64C0 adds the state to sp+I;
//   the checked index carousel+4h into the vector sp+144h (begin +148h, end
//   +14Ch; 00BF6713 when begin is null or index >= (end - begin) >> 2, SAR,
//   then the read anyway) gives a record whose +4h key looks up a word in the
//   map sp+1B4h (00617870), and sp+I vcall +88h(word, 0, 1.0f);
//   "circle_HL_Icon" (00CF4DC0) under sp+C to sp+H, hidden (vcall +34h(0));
//   "circle_small_01_Group" (00CF4DA8) and "circle_small_02_Group" (00CF4D90)
//   under sp+C hidden.
// - Positions (vcall +44h, one float): up: sp+54h gets bc+2A8h * 5.0 (double
//   00D7A370) - bc+2A4h and sp+34h the same with +, each x87 and stored to a
//   float local before the push; down: sp+48h gets -0.0f (00D7A208) - bc+2A4h
//   (SUBSS) and sp+28h bc+2A4h.
// - The fourth section S = 44h up ("circle_full_3_Section", 00CF4D78) / 38h
//   down ("circle_full_0_Section", 00CF4D60): sp+S hidden, 00AA7E00(sp+C,
//   name, 1) to sp+S, shown, vcall +4Ch(0.5f); 0065E870(ECX = bc, sp+C,
//   0.0f); carousel+40h = 1.0f (00D7A24C, MOVSS) before the name's release.
// No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_powerup_carousel_rebuild_slots_006179d0(void* carousel, Cc13SweepB9Host& host) {
    auto sp = [&]() { return at<void*>(in_game_gui(host), 0xC0); };
    auto bc = [&]() { return at<void*>(in_game_gui(host), 0xBC); };
    const float half = f32_bits(0x3F000000u);  // 00CE3800

    at<std::uint8_t>(carousel, 0x3F) = 1;
    host.widget_vcall_34_set_visible(at<void*>(bc(), 0x100), 0);
    host.widget_vcall_34_set_visible(at<void*>(bc(), 0x104), 0);

    const bool up = at<std::uint8_t>(carousel, 0x3C) == 0;
    if (up) {
        at<std::uint32_t>(sp(), 0x21C) = at<std::uint32_t>(sp(), 0x28);
        at<std::uint32_t>(sp(), 0x220) = at<std::uint32_t>(sp(), 0x48);
    } else {
        at<std::uint32_t>(sp(), 0x21C) = at<std::uint32_t>(sp(), 0x34);
        at<std::uint32_t>(sp(), 0x220) = at<std::uint32_t>(sp(), 0x54);
    }

    // The shift: up walks off = 38h..40h and copies from off + 4 (n = 0..2);
    // down walks off = 44h..3Ch and copies from off - 4 (n = 3..1).
    const std::int32_t step = up ? 4 : -4;
    std::size_t off = up ? 0x38 : 0x44;
    std::int32_t n = up ? 0 : 3;
    for (int pass = 0; pass < 3; ++pass, off += step, n += (up ? 1 : -1)) {
        at<std::uint32_t>(sp(), off - 0x10) = at<std::uint32_t>(sp(), off - 0x10 + step);
        at<std::uint32_t>(sp(), off + 0x40) = at<std::uint32_t>(sp(), off + 0x40 + step);
        at<std::uint32_t>(sp(), off + 0x10) = at<std::uint32_t>(sp(), off + 0x10 + step);

        Cc13SweepBNativeString suffix = {0, nullptr};
        host.native_string_resize_0041dd40(&suffix, 8, 1);
        if (suffix.data != nullptr) host.crt_memcpy_00bf7680(suffix.data, "_Section", suffix.length + 1);  // 00CF4C5C
        // Constructed by the callees (uninitialised stack slots in the image).
        Cc13SweepBNativeString number = {0, nullptr};
        Cc13SweepBNativeString prefixed = {0, nullptr};
        Cc13SweepBNativeString name = {0, nullptr};
        void* number_out = host.native_string_from_int_004260b0(&number, n);
        void* prefixed_out = host.native_string_prefixed_0043c130(&prefixed, "circle_full_", number_out);  // 00CF4C4C
        host.native_string_concat_004261a0(prefixed_out, &name, &suffix);
        release_string(prefixed, host);
        release_string(number, host);
        release_string(suffix, host);

        host.widget_vcall_34_set_visible(at<void*>(sp(), off), 0);
        at<void*>(sp(), off) = host.widget_find_child_00aa7e00(at<void*>(sp(), off - 0x10), &name, 1);
        host.widget_vcall_34_set_visible(at<void*>(sp(), off), 1);
        host.widget_vcall_4c_set_float(at<void*>(sp(), off), half);
        release_string(name, host);
    }

    const std::size_t clone_slot = up ? 0x34 : 0x28;
    const std::size_t icon_slot = up ? 0x54 : 0x48;
    const std::size_t highlight_slot = up ? 0x84 : 0x78;

    at<void*>(sp(), clone_slot) = host.widget_clone_subtree_00aab4c0(at<void*>(bc(), 0x138), nullptr);
    host.widget_vcall_34_set_visible(at<void*>(sp(), clone_slot), 1);
    with_c_string("icon_L_Icon", 0xB, host, [&](Cc13SweepBNativeString* name) {  // 00CF4DDC
        at<void*>(sp(), icon_slot) = host.widget_find_child_00aa7e00(at<void*>(sp(), clone_slot), name, 1);
    });
    with_c_string("number_Text", 0xB, host, [&](Cc13SweepBNativeString* name) {  // 00CF4DD0
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(sp(), icon_slot), name, 1), 0);
    });
    host.widget_vcall_88(at<void*>(sp(), icon_slot), 3, 0, 1.0f);

    const std::uint32_t start =
        static_cast<std::uint32_t>(static_cast<std::int32_t>(host.widget_vcall_98_short(at<void*>(sp(), icon_slot))));
    for (std::uint32_t i = start; i < at<std::uint32_t>(sp(), 0x1C8) + start; ++i) {
        std::int32_t key_a = static_cast<std::int32_t>(i);
        std::int32_t key_b = static_cast<std::int32_t>(i);
        void** first = host.map_subscript_006170f0(static_cast<std::uint8_t*>(sp()) + 0x1C0, &key_a);
        void** second = host.map_subscript_006170f0(static_cast<std::uint8_t*>(sp()) + 0x1C0, &key_b);
        host.gui_icon_add_state_00ab64c0(at<void*>(sp(), icon_slot), at<void*>(*second, 0xD0),
                                         static_cast<std::uint8_t*>(*first) + 0xD4);
    }

    const std::uint32_t index = at<std::uint32_t>(carousel, 0x4);
    void** begin = at<void**>(sp(), 0x148);
    if (begin == nullptr ||
        index >= static_cast<std::uint32_t>(
                     static_cast<std::int32_t>(at<std::uintptr_t>(sp(), 0x14C) -
                                               reinterpret_cast<std::uintptr_t>(begin)) >> 2)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* record = at<void**>(sp(), 0x148)[index];
    const std::uint16_t state = *static_cast<const std::uint16_t*>(
        host.string_map_ci_subscript_00617870(static_cast<std::uint8_t*>(sp()) + 0x1B4, at<void*>(record, 0x4)));
    host.widget_vcall_88(at<void*>(sp(), icon_slot), state, 0, 1.0f);

    with_c_string("circle_HL_Icon", 0xE, host, [&](Cc13SweepBNativeString* name) {  // 00CF4DC0
        at<void*>(sp(), highlight_slot) = host.widget_find_child_00aa7e00(at<void*>(sp(), clone_slot), name, 1);
    });
    host.widget_vcall_34_set_visible(at<void*>(sp(), highlight_slot), 0);
    with_c_string("circle_small_01_Group", 0x15, host, [&](Cc13SweepBNativeString* name) {  // 00CF4DA8
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(sp(), clone_slot), name, 1), 0);
    });
    with_c_string("circle_small_02_Group", 0x15, host, [&](Cc13SweepBNativeString* name) {  // 00CF4D90
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(sp(), clone_slot), name, 1), 0);
    });

    if (up) {
        const double five = f64_bits(0x4014000000000000ull);  // 00D7A370
        const float icon_pos =
            pc24(static_cast<double>(pc24(static_cast<double>(at<float>(bc(), 0x2A8)) * five)) -
                 static_cast<double>(at<float>(bc(), 0x2A4)));
        host.widget_vcall_44_set_float(at<void*>(sp(), 0x54), icon_pos);
        const float clone_pos =
            pc24(static_cast<double>(pc24(static_cast<double>(at<float>(bc(), 0x2A8)) * five)) +
                 static_cast<double>(at<float>(bc(), 0x2A4)));
        host.widget_vcall_44_set_float(at<void*>(sp(), 0x34), clone_pos);
    } else {
        host.widget_vcall_44_set_float(at<void*>(sp(), 0x48), f32_bits(0x80000000u) - at<float>(bc(), 0x2A4));  // 00D7A208
        host.widget_vcall_44_set_float(at<void*>(sp(), 0x28), at<float>(bc(), 0x2A4));
    }

    const std::size_t section_slot = up ? 0x44 : 0x38;
    Cc13SweepBNativeString section = {0, nullptr};
    host.native_string_resize_0041dd40(&section, 0x15, 1);
    if (section.data != nullptr) {
        host.crt_memcpy_00bf7680(section.data, up ? "circle_full_3_Section" : "circle_full_0_Section",  // 00CF4D78 / 00CF4D60
                                 section.length + 1);
    }
    host.widget_vcall_34_set_visible(at<void*>(sp(), section_slot), 0);
    at<void*>(sp(), section_slot) = host.widget_find_child_00aa7e00(at<void*>(sp(), clone_slot), &section, 1);
    host.widget_vcall_34_set_visible(at<void*>(sp(), section_slot), 1);
    host.widget_vcall_4c_set_float(at<void*>(sp(), section_slot), half);
    host.hud_support_set_large_icon_alpha_0065e870(bc(), at<void*>(sp(), clone_slot), 0.0f);
    at<float>(carousel, 0x40) = f32_bits(0x3F800000u);  // 00D7A24C
    release_string(section, host);
}

// ---------------------------------------------------------------------------
// 0056F320 BSP_MultiSessionBrowserScreen_Register
// __thiscall(ECX = screen), RET; vtable 00CEF134 slot +10h. EH frame 00C6EE61.
// Image facts (asm 0056F320..005702FB):
// - 004F71D0(screen); +50h = 0; screen vtable +14h().
// - The scene records: cont0 = [[00E198AC]+5Ch] + 24h, cur = [cont0+4h];
//   00BF6713 when cur > [cont0+8h] (JBE). Each pass re-reads cont =
//   [[00E198AC]+5Ch] + 24h: 00BF6713 when [cont+4h] > [cont+8h] (JBE) and
//   when cont != cont0; the loop ends when cur == [cont+8h].
// - Body: a temporary "/" (00CE7898, length 1, at [ESP+58h]); two checked
//   dereferences (00BF6713 unless cur < [cont0+8h], JC); pos =
//   00467CF0(cur+20h, "/", 7FFFFFFFh); 00469840(cur+20h, file, pos + 1,
//   7FFFFFFFh) (the count is the first 7FFFFFFFh push, left on the stack by
//   00467CF0's RET 8); "/" released; n = strstr(file.data, ".scn" 00CE7888)
//   - file.data, or -1 when file.data or the match is null; tmp =
//   00469840(file, tmp, 0, n); unless the result is &file: 0041DD40(file,
//   result.length, 1) and, when result.length != 0, memcpy(file.data,
//   result.data, file.length) (no terminator byte); tmp released.
// - Eight texture preloads, suffixes _C.tga (00CEF0B0), _D.tga (00CEF098),
//   _E.tga (00CEF090), _ICH.tga (00CEF084), _ICL.tga (00CEF078), _ICM.tga
//   (00CEF06C), _ICS.tga (00CEF060), _S.tga (00CEF058): suffix temporary;
//   sub = 00469840(file, sub, 0, 14h); prefixed = 0043C130("gui/radarshots/"
//   00CEF0A0, sub); name = 004261A0(prefixed, name, suffix); a local rect
//   {0.0f, 0.0f, 1.0f, 1.0f} (XORPS / 00D7A24C, MOVSS); +A0h vtable
//   +A4h(name, rect, 0, 0); released name, prefixed, sub, suffix.
// - file released; 00BF6713 unless cur < [cont0+8h] (JC); cur += 434h.
// - After the loop (the PUSH 1 at 0056F3A6, taken before the exit JZ, is
//   00ABBE50's second argument): 00ABBE50(+4Ch, "globals.no_server_found"
//   00CEF040, 1); +4Ch vcall +34h(0); p = 00AA6750(+F4h, buf): +104h/+108h/
//   +10Ch = p[0..2]; +25Ch = 00AA6750(+F8h)[1] - 00AA6750(+F4h)[1]; +260h =
//   00AA6750(+98h)[1] - 00AA6750(+94h)[1] (each one x87 FSUB of floats,
//   stored as float); 0056D9A0(screen); 005684D0(screen); +94h and +98h
//   vcall +34h(0).
// - [00E086C0] = -1; +244h/+248h/+24Ch = [00E188A8]+670h/+674h/+678h.
// - The list [00E188A8]+664h (head node [+668h]): node = head->next; per
//   pass 00BF6713 when the list object moved, the end test against the
//   re-read head, then +10h+i = node+8h and +19h+i = node+8h (bytes; each
//   read checked against the head, 00BF6713), node = node->next, ++i. No
//   bound on i in the image.
// - +250h = +254h = +258h = 0; bytes +F0h = +F1h = 0.
// No draw from the shared RNG 00BD2F10. The x87 subtractions are single
// operations of float operands, so 24-bit precision gives float(a - b).
// ---------------------------------------------------------------------------
void cc13_sweepB9_session_browser_register_0056f320(void* screen, Cc13SweepB9Host& host) {
    host.frontend_screen_register_004f71d0(screen);
    at<std::uint8_t>(screen, 0x50) = 0;
    host.screen_vcall_14(screen);

    auto scene_container = [&]() {
        return static_cast<std::uint8_t*>(at<void*>(*static_cast<void**>(host.image_global(0x00E198AC)), 0x5C)) +
               0x24;
    };
    std::uint8_t* const cont0 = scene_container();
    std::uint8_t* cur = at<std::uint8_t*>(cont0, 0x4);
    if (cur > at<std::uint8_t*>(cont0, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
    static const char* const kSuffixes[8] = {"_C.tga",   "_D.tga",   "_E.tga",   "_ICH.tga",
                                             "_ICL.tga", "_ICM.tga", "_ICS.tga", "_S.tga"};
    static const std::uint32_t kSuffixLengths[8] = {6, 6, 6, 8, 8, 8, 8, 6};
    for (;;) {
        std::uint8_t* const cont = scene_container();
        std::uint8_t* const end = at<std::uint8_t*>(cont, 0x8);
        if (at<std::uint8_t*>(cont, 0x4) > end) host.crt_invalid_parameter_noinfo_00bf6713();
        if (cont0 != cont) host.crt_invalid_parameter_noinfo_00bf6713();
        if (cur == end) break;

        Cc13SweepBNativeString slash = {0, nullptr};
        host.native_string_resize_0041dd40(&slash, 1, 1);
        if (slash.data != nullptr) host.crt_memcpy_00bf7680(slash.data, "/", slash.length + 1);  // 00CE7898
        if (!(cur < at<std::uint8_t*>(cont0, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        if (!(cur < at<std::uint8_t*>(cont0, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        Cc13SweepBNativeString file = {0, nullptr};  // constructed by 00469840
        const std::int32_t pos = host.native_string_rfind_00467cf0(cur + 0x20, &slash, 0x7FFFFFFFu);
        host.native_string_substring_00469840(cur + 0x20, &file, pos + 1, 0x7FFFFFFF);
        release_string(slash, host);

        std::int32_t count = -1;
        if (file.data != nullptr) {
            const char* match = host.crt_strstr_00bf9440(static_cast<const char*>(file.data), ".scn");  // 00CE7888
            if (match != nullptr) count = static_cast<std::int32_t>(match - static_cast<const char*>(file.data));
        }
        Cc13SweepBNativeString tmp = {0, nullptr};  // constructed by 00469840
        auto* result = static_cast<Cc13SweepBNativeString*>(host.native_string_substring_00469840(&file, &tmp, 0, count));
        if (&file != result) {
            host.native_string_resize_0041dd40(&file, result->length, 1);
            if (result->length != 0) host.crt_memcpy_00bf7680(file.data, result->data, file.length);
        }
        release_string(tmp, host);

        for (int k = 0; k < 8; ++k) {
            Cc13SweepBNativeString suffix = {0, nullptr};
            host.native_string_resize_0041dd40(&suffix, kSuffixLengths[k], 1);
            if (suffix.data != nullptr) host.crt_memcpy_00bf7680(suffix.data, kSuffixes[k], suffix.length + 1);
            Cc13SweepBNativeString sub = {0, nullptr};
            Cc13SweepBNativeString prefixed = {0, nullptr};
            Cc13SweepBNativeString name = {0, nullptr};
            void* sub_out = host.native_string_substring_00469840(&file, &sub, 0, 0x14);
            void* prefixed_out = host.native_string_prefixed_0043c130(&prefixed, "gui/radarshots/", sub_out);  // 00CEF0A0
            void* name_out = host.native_string_concat_004261a0(prefixed_out, &name, &suffix);
            const float rect[4] = {0.0f, 0.0f, f32_bits(0x3F800000u), f32_bits(0x3F800000u)};  // 00D7A24C
            host.texture_owner_vcall_a4(at<void*>(screen, 0xA0), name_out, rect, 0, 0);
            release_string(name, host);
            release_string(prefixed, host);
            release_string(sub, host);
            release_string(suffix, host);
        }
        release_string(file, host);
        if (!(cur < at<std::uint8_t*>(cont0, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        cur += 0x434;
    }

    host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, 0x4C), "globals.no_server_found", 1);  // 00CEF040
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x4C), 0);
    float buf[3];
    const float* p = host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0xF4), buf);
    at<float>(screen, 0x104) = p[0];
    at<float>(screen, 0x108) = p[1];
    at<float>(screen, 0x10C) = p[2];
    const float top_a = host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0xF8), buf)[1];
    float buf2[3];
    at<float>(screen, 0x25C) =
        pc24(static_cast<double>(top_a) -
             static_cast<double>(host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0xF4), buf2)[1]));
    float buf3[3];
    const float top_b = host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x98), buf3)[1];
    float buf4[3];
    at<float>(screen, 0x260) =
        pc24(static_cast<double>(top_b) -
             static_cast<double>(host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x94), buf4)[1]));
    cc13_sweepB3_session_browser_build_filter_rows_0056d9a0(screen, host);
    host.session_browser_build_create_options_005684d0(screen);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x94), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x98), 0);

    *static_cast<std::int32_t*>(host.image_global(0x00E086C0)) = -1;
    at<std::uint32_t>(screen, 0x244) = at<std::uint32_t>(host.global_00e188a8(), 0x670);
    at<std::uint32_t>(screen, 0x248) = at<std::uint32_t>(host.global_00e188a8(), 0x674);
    at<std::uint32_t>(screen, 0x24C) = at<std::uint32_t>(host.global_00e188a8(), 0x678);

    std::uint8_t* const list0 = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x664;
    void* node = *static_cast<void**>(at<void*>(list0, 0x4));
    for (std::uint32_t i = 0;; ++i) {
        std::uint8_t* const list = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x664;
        void* const head = at<void*>(list, 0x4);
        if (list0 != list) host.crt_invalid_parameter_noinfo_00bf6713();
        if (node == head) break;
        if (node == at<void*>(list0, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        at<std::uint8_t>(screen, 0x10 + i) = at<std::uint8_t>(node, 0x8);
        if (node == at<void*>(list0, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        at<std::uint8_t>(screen, 0x19 + i) = at<std::uint8_t>(node, 0x8);
        if (node == at<void*>(list0, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        node = *static_cast<void**>(node);
    }
    at<std::uint32_t>(screen, 0x250) = 0;
    at<std::uint32_t>(screen, 0x254) = 0;
    at<std::uint32_t>(screen, 0x258) = 0;
    at<std::uint8_t>(screen, 0xF0) = 0;
    at<std::uint8_t>(screen, 0xF1) = 0;
}

// ---------------------------------------------------------------------------
// 0067F4D0 BSP_UnitLibraryScreen_Enter
// __thiscall(ECX = screen), RET; vtable 00CF6E50 slot +18h. EH frame 00C7D2D9.
// Image facts (asm 0067F4D0..0068051D):
// - When +E0h == 0: p = operator new(B8h) (00BF681B); env = p ? 0078DAA0(p,
//   +0Ch, 0) : 0; +E0h = env. Then a 288h-byte stack block at [ESP+30h]:
//   004CB420(block); 0078C9B0(ECX = +E0h, &block) (called even when env is
//   null); 004C6520(block).
// - 00518D60(ECX = 0Bh, EDX = 0, 0, "UNITLIB" 00CF70BC); 0054A0C0(ECX =
//   [00E1930C], "Select a unit with left and right" 00CF7098).
// - 00AA7E00(parent, name, 1) for each child, the result stored before the
//   name's release: "Main_Listbox" under +6Ch to +70h, then 00A9C7C0(+70h, 0),
//   00A9AC40(+70h, screen+64h), +70h vcall +60h(1) and +80h(1); under +6Ch
//   Pic_Icon +98h, PicPrev_Icon +9Ch, PicNext_Icon +A0h, NyilPrev_Icon +A4h,
//   NyilNext_Icon +A8h, GroupNyilPrev_Icon +ACh, GroupNyilNext_Icon +B0h,
//   each followed (after the release) by 00AA6BC0(widget, screen+68h, 0);
//   under +6Ch UnitName_Text +B4h, UnitNamePrev_Text +B8h,
//   UnitNameNext_Text +BCh, UnitLocked_Text +C0h, UnitLockedPrev_Text +C4h,
//   UnitLockedNext_Text +C8h, Details_Group +78h; under +78h UnitMovie_Movie
//   +94h, Description_Group +7Ch; +80h = +7Ch / "AlapIze_Group" /
//   "RedTitle_Group" / "Title_Text" (three chained finds; the names are
//   built Title, RedTitle, AlapIze and released AlapIze, RedTitle, Title);
//   under +7Ch DescriptionUL_Text +84h, DescriptionUR_Text +88h,
//   DescriptionB_Text +8Ch, DescriptionBB_Text +90h; +78h vcall +34h(0);
//   under +78h Description_Listbox +74h; under +70h GroupName_Text +CCh,
//   FlagUSLeft_AnimIcon +D0h, FlagJapLeft_AnimIcon +D4h,
//   FlagUSRight_AnimIcon +D8h, FlagJapRight_AnimIcon +DCh.
// - +E8h = 0.
// No x87 arithmetic, no draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_unit_library_enter_0067f4d0(void* screen, Cc13SweepB9Host& host) {
    if (at<void*>(screen, 0xE0) == nullptr) {
        void* storage = host.crt_operator_new_00bf681b(0xB8);
        void* env = nullptr;
        if (storage != nullptr) {
            env = cc13_sweepB_ocean_environment_construct_0078daa0(storage, at<void*>(screen, 0xC), 0, host);
        }
        at<void*>(screen, 0xE0) = env;
        alignas(8) std::uint8_t block[0x288];
        host.scene_environment_block_construct_004cb420(block);
        host.ocean_environment_apply_block_0078c9b0(at<void*>(screen, 0xE0), block);
        host.scene_environment_block_destruct_004c6520(block);
    }
    with_c_string("UNITLIB", 7, host, [&](Cc13SweepBNativeString* text) {  // 00CF70BC
        host.frontend_frame_select_00518d60(0xB, 0, 0, text);
    });
    with_c_string("Select a unit with left and right", 0x21, host, [&](Cc13SweepBNativeString* text) {  // 00CF7098
        host.main_menu_command_bar_set_help_line_0054a0c0(*static_cast<void**>(host.image_global(0x00E1930C)), text);
    });

    auto bind = [&](std::size_t parent, const char* name, std::uint32_t length, std::size_t slot) {
        with_c_string(name, length, host, [&](Cc13SweepBNativeString* text) {
            at<void*>(screen, slot) = host.widget_find_child_00aa7e00(at<void*>(screen, parent), text, 1);
        });
    };

    bind(0x6C, "Main_Listbox", 0xC, 0x70);  // 00CEB334
    host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x70), 0);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x70), static_cast<std::uint8_t*>(screen) + 0x64);
    host.widget_vcall_60(at<void*>(screen, 0x70), 1);
    host.widget_vcall_80(at<void*>(screen, 0x70), 1);

    struct Child {
        std::size_t parent;
        const char* name;
        std::uint32_t length;
        std::size_t slot;
    };
    static const Child kListened[] = {
        {0x6C, "Pic_Icon", 0x8, 0x98},             {0x6C, "PicPrev_Icon", 0xC, 0x9C},       // 00CF708C 00CF707C
        {0x6C, "PicNext_Icon", 0xC, 0xA0},         {0x6C, "NyilPrev_Icon", 0xD, 0xA4},      // 00CF706C 00CF705C
        {0x6C, "NyilNext_Icon", 0xD, 0xA8},        {0x6C, "GroupNyilPrev_Icon", 0x12, 0xAC},  // 00CF704C 00CF7038
        {0x6C, "GroupNyilNext_Icon", 0x12, 0xB0},  // 00CF7024
    };
    for (const Child& child : kListened) {
        bind(child.parent, child.name, child.length, child.slot);
        host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, child.slot), static_cast<std::uint8_t*>(screen) + 0x68,
                                              0);
    }
    static const Child kPlain[] = {
        {0x6C, "UnitName_Text", 0xD, 0xB4},         {0x6C, "UnitNamePrev_Text", 0x11, 0xB8},    // 00CF7014 00CF7000
        {0x6C, "UnitNameNext_Text", 0x11, 0xBC},    {0x6C, "UnitLocked_Text", 0xF, 0xC0},       // 00CF6FEC 00CF6FDC
        {0x6C, "UnitLockedPrev_Text", 0x13, 0xC4},  {0x6C, "UnitLockedNext_Text", 0x13, 0xC8},  // 00CF6FC8 00CF6FB4
        {0x6C, "Details_Group", 0xD, 0x78},         {0x78, "UnitMovie_Movie", 0xF, 0x94},       // 00CF6FA4 00CF6F94
        {0x78, "Description_Group", 0x11, 0x7C},    // 00CF6F80
    };
    for (const Child& child : kPlain) bind(child.parent, child.name, child.length, child.slot);

    // +80h: three chained finds under +7Ch.
    with_c_string("Title_Text", 0xA, host, [&](Cc13SweepBNativeString* title) {  // 00CF6F74
        with_c_string("RedTitle_Group", 0xE, host, [&](Cc13SweepBNativeString* red_title) {  // 00CF6F64
            with_c_string("AlapIze_Group", 0xD, host, [&](Cc13SweepBNativeString* base_group) {  // 00CF6F54
                void* group = host.widget_find_child_00aa7e00(at<void*>(screen, 0x7C), base_group, 1);
                void* red = host.widget_find_child_00aa7e00(group, red_title, 1);
                at<void*>(screen, 0x80) = host.widget_find_child_00aa7e00(red, title, 1);
            });
        });
    });

    static const Child kDescription[] = {
        {0x7C, "DescriptionUL_Text", 0x12, 0x84},  {0x7C, "DescriptionUR_Text", 0x12, 0x88},  // 00CF6F40 00CF6F2C
        {0x7C, "DescriptionB_Text", 0x11, 0x8C},   {0x7C, "DescriptionBB_Text", 0x12, 0x90},  // 00CF6F18 00CF6F04
    };
    for (const Child& child : kDescription) bind(child.parent, child.name, child.length, child.slot);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x78), 0);

    static const Child kTail[] = {
        {0x78, "Description_Listbox", 0x13, 0x74},    {0x70, "GroupName_Text", 0xE, 0xCC},          // 00CF6EF0 00CF6EE0
        {0x70, "FlagUSLeft_AnimIcon", 0x13, 0xD0},    {0x70, "FlagJapLeft_AnimIcon", 0x14, 0xD4},   // 00CF6ECC 00CF6EB4
        {0x70, "FlagUSRight_AnimIcon", 0x14, 0xD8},   {0x70, "FlagJapRight_AnimIcon", 0x15, 0xDC},  // 00CF6E9C 00CF6E84
    };
    for (const Child& child : kTail) bind(child.parent, child.name, child.length, child.slot);
    at<std::uint32_t>(screen, 0xE8) = 0;
}

// ---------------------------------------------------------------------------
// 005D73A0 BSP_MultiGameLobbyScreen_RefreshPlayerRows
// __fastcall(ECX = screen), RET. EH frame 00C7412A. g = [00E188A8] (re-read
// at every access, as the image does); rec(i) = [g+18CCh+i*4].
// Image facts (asm 005D73A0..005D8401):
// - local = rec([g+18ECh]): +148h vcall +34h(local+28h == 0), +14Ch vcall
//   +34h(local+28h == 1) (no null check; local re-read each time); the third
//   read is kept ([ESP+B4h]) and when non-null with local+28h < 2 (signed)
//   00518B90(ECX = local+28h). q0 = 00786010(ECX = g+1EF8h, [g+18C8h]),
//   stored as float.
// - Rows i = 0..7: rec = rec(i), row = screen+250h+i*4. Children of row:
//   OutofControl_Group (og), Connection_Group (conn); row vcall +34h(1);
//   row+77h = 0; flag_US_Icon, flag_JP_Icon, player_tag_Text (tag),
//   player_name_Text (name); of og: rank_Icon, droprate_Icon, speak_Icon; of
//   row: check_Icon. flag_US vcall +34h(rec+20h >= 0 && rec+28h == 0),
//   flag_JP vcall +34h(rec+20h >= 0 && rec+28h == 1) (signed JL).
// - text = {0, 0}; tag_text = " " (00CE3A90).
// - rec+8h != 0 and rec+9h == 0 (joined): droprate hidden; check vcall
//   +34h(0) when byte g+218Ch, else +34h(byte rec+0Ch) (the push is a dword
//   whose low byte is rec+0Ch); text = "." (00CE3A70) + the C string rec+58h
//   (004261A0, inline assignment: 0041DD40(len, 1) and memcpy of len
//   bytes); when _stricmp(text, ".globals.newplayer") == 0 (00BF7FBF, only
//   with text.data non-null): 0041DD40(text, 11h, 0) and memcpy of 11h
//   bytes of "globals.newplayer" (00CEF794); when the C string rec+78h has 1
//   to 3 characters tag_text = it (0041E870 temporary, inline assignment);
//   rank vcall +34h(byte g+218Ch == 0); 00517CB0(ECX = rank, EDX = rec+90h);
//   speak vcall +34h(0); when byte g+218Ch == 0 and (rec+89h != 0 or
//   (rec+94h == 2 and 007F74B0(rec))): speak vcall +34h(1), then speak
//   vcall +88h(2, 0, 1.0f) when local is non-null, local != rec and
//   00778740(g+2080h, local+80h, local+84h, rec+80h, rec+84h); else
//   +88h(1, 0, 1.0f) when 007F74B0(rec), else +88h(0, 0, 1.0f) when rec+94h
//   == 1, else +34h(0). conn vcall +34h(byte g+218Ch == 0); ping =
//   rec+1Ch == 0 ? q0 : (00786010(g+1EF8h, rec+1Ch) + q0) * 0.5 (double
//   00D7A280); ping + 1.0e-5 (double 00D7A330) stored as float;
//   00578580(conn, ping). When (g+2060h:2064h & rec+D0h:D4h) == g+2060h:
//   2064h, name and tag vcall +50h(&{[00CEF16C], [00CEF168], 0.6f
//   (00CE3D30), 1.0f}) (a fresh local each); else both +50h(00E195F0), the
//   static {1, 0, 0, 1} under guard bit 0 of 00E19600, checked before each.
// - rec+8h == 0 and rec+9h == 0 (empty): text = C string [00E088F8]
//   (0041DD40(len, 0), memcpy of len bytes); name and tag +50h(00E195DC),
//   the static {1, 1, 1, 1} under guard bit 0 of 00E195EC.
// - rec+9h != 0 (either rec+8h): when rec+0Ah != 0, text = C string
//   [00E08940 + rec+2Ch*4] and name, tag +50h(the 00CEF16C colour);
//   otherwise text = [00E088F8] and name, tag +50h(00E195DC); then rank,
//   droprate, check, speak and conn vcall +34h(0).
// - All paths: when i == [00E08958]: equal = [00E195D4] == 0 ? text.length
//   == 0 : (text.length != 0 && _stricmp([00E195D8], text.data) == 0);
//   when equal, text = C string [00E088FC], else [00E08958] = -1.
//   00ABAED0(name, &text, 1); 00ABAED0(tag, &tag_text, 1); when g+5FCh != 0
//   and i > 8 (never true for i < 8), row+77h = 1; tag_text, text released.
// x87 (rule 4c, main thread): the ping sum, halving and bias are three
// float-rounded operations. No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_lobby_refresh_player_rows_005d73a0(void* screen, Cc13SweepB9Host& host) {
    auto g = [&]() { return host.global_00e188a8(); };
    auto record = [&](std::uint32_t index) { return at<void*>(g(), 0x18CC + index * 4); };
    auto local_record = [&]() { return record(at<std::uint32_t>(g(), 0x18EC)); };
    auto cstr = [&](std::uint32_t address) { return *static_cast<const char* const*>(host.image_global(address)); };
    auto assign_cstr = [&](Cc13SweepBNativeString& text, const char* source) {
        const std::uint32_t length = source != nullptr ? static_cast<std::uint32_t>(std::strlen(source)) : 0;
        host.native_string_resize_0041dd40(&text, length, 0);
        if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length);
    };
    auto assign_string = [&](Cc13SweepBNativeString& text, const Cc13SweepBNativeString* source) {
        if (&text != source) {
            host.native_string_resize_0041dd40(&text, source->length, 1);
            if (source->length != 0) host.crt_memcpy_00bf7680(text.data, source->data, text.length);
        }
    };
    auto static_colour = [&](std::uint32_t guard_address, std::uint32_t colour_address, float r, float gr, float b,
                             float a) {
        auto& guard = *static_cast<std::uint32_t*>(host.image_global(guard_address));
        if ((guard & 1u) == 0) {
            guard |= 1u;
            auto* colour = static_cast<float*>(host.image_global(colour_address));
            colour[0] = r;
            colour[1] = gr;
            colour[2] = b;
            colour[3] = a;
        }
        return static_cast<const float*>(host.image_global(colour_address));
    };
    const float one = f32_bits(0x3F800000u);  // 00D7A24C
    auto highlight = [&](void* widget) {
        const float colour[4] = {*static_cast<const float*>(host.image_global(0x00CEF16C)),
                                 *static_cast<const float*>(host.image_global(0x00CEF168)),
                                 f32_bits(0x3F19999Au) /* 00CE3D30 */, one};
        host.widget_vcall_50_colour(widget, colour);
    };

    host.widget_vcall_34_set_visible(at<void*>(screen, 0x148), at<std::int32_t>(local_record(), 0x28) == 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x14C), at<std::int32_t>(local_record(), 0x28) == 1);
    void* const local = local_record();
    if (local != nullptr && at<std::int32_t>(local, 0x28) < 2) {
        host.frontend_frame_set_flag_00518b90(at<std::int32_t>(local, 0x28));
    }
    const float q0 = host.game_peer_rows_get_row_quality_00786010(static_cast<std::uint8_t*>(g()) + 0x1EF8,
                                                                  at<std::uint32_t>(g(), 0x18C8));

    for (std::int32_t i = 0; i < 8; ++i) {
        void* const rec = record(static_cast<std::uint32_t>(i));
        void* const row = at<void*>(screen, 0x250 + static_cast<std::size_t>(i) * 4);
        auto find = [&](void* parent, const char* name, std::uint32_t length) {
            void* found = nullptr;
            with_c_string(name, length, host, [&](Cc13SweepBNativeString* text) {
                found = host.widget_find_child_00aa7e00(parent, text, 1);
            });
            return found;
        };
        void* const out_of_control = find(row, "OutofControl_Group", 0x12);  // 00CF214C
        void* const connection = find(row, "Connection_Group", 0x10);        // 00CEF170
        host.widget_vcall_34_set_visible(row, 1);
        at<std::uint8_t>(row, 0x77) = 0;
        void* const flag_us = find(row, "flag_US_Icon", 0xC);                 // 00CF213C
        void* const flag_jp = find(row, "flag_JP_Icon", 0xC);                 // 00CF212C
        void* const tag = find(row, "player_tag_Text", 0xF);                  // 00CF211C
        void* const name = find(row, "player_name_Text", 0x10);               // 00CF2108
        void* const rank = find(out_of_control, "rank_Icon", 0x9);            // 00CEEE30
        void* const droprate = find(out_of_control, "droprate_Icon", 0xD);    // 00CF20F8
        void* const speak = find(out_of_control, "speak_Icon", 0xA);          // 00CF20EC
        void* const check = find(row, "check_Icon", 0xA);                     // 00CF20E0
        host.widget_vcall_34_set_visible(flag_us, at<std::int32_t>(rec, 0x20) >= 0 && at<std::int32_t>(rec, 0x28) == 0);
        host.widget_vcall_34_set_visible(flag_jp, at<std::int32_t>(rec, 0x20) >= 0 && at<std::int32_t>(rec, 0x28) == 1);

        Cc13SweepBNativeString text = {0, nullptr};
        Cc13SweepBNativeString tag_text = {0, nullptr};
        host.native_string_resize_0041dd40(&tag_text, 1, 1);
        if (tag_text.data != nullptr) host.crt_memcpy_00bf7680(tag_text.data, " ", tag_text.length + 1);  // 00CE3A90

        const bool joined = at<std::uint8_t>(rec, 0x8) != 0;
        const bool flag9 = at<std::uint8_t>(rec, 0x9) != 0;
        if (joined && !flag9) {
            host.widget_vcall_34_set_visible(droprate, 0);
            if (at<std::uint8_t>(g(), 0x218C) != 0) {
                host.widget_vcall_34_set_visible(check, 0);
            } else {
                host.widget_vcall_34_set_visible(check, at<std::uint8_t>(rec, 0xC));
            }
            const char* const player_name = static_cast<const char*>(rec) + 0x58;
            Cc13SweepBNativeString name_part = {0, nullptr};
            host.native_string_resize_0041dd40(&name_part, static_cast<std::uint32_t>(std::strlen(player_name)), 1);
            if (name_part.data != nullptr) host.crt_memcpy_00bf7680(name_part.data, player_name, name_part.length + 1);
            Cc13SweepBNativeString dot = {0, nullptr};
            host.native_string_resize_0041dd40(&dot, 1, 1);
            if (dot.data != nullptr) host.crt_memcpy_00bf7680(dot.data, ".", dot.length + 1);  // 00CE3A70
            Cc13SweepBNativeString joined_text = {0, nullptr};  // constructed by 004261A0
            auto* result =
                static_cast<Cc13SweepBNativeString*>(host.native_string_concat_004261a0(&dot, &joined_text, &name_part));
            assign_string(text, result);
            release_string(joined_text, host);
            release_string(dot, host);
            release_string(name_part, host);
            if (text.data != nullptr &&
                host.crt_stricmp_00bf7fbf(static_cast<const char*>(text.data), ".globals.newplayer") == 0) {  // 00CF20CC
                host.native_string_resize_0041dd40(&text, 0x11, 0);
                if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, "globals.newplayer", text.length);  // 00CEF794
            }
            const char* const player_tag = static_cast<const char*>(rec) + 0x78;
            if (std::strlen(player_tag) != 0 && std::strlen(player_tag) < 4) {
                Cc13SweepBNativeString tag_tmp = {0, nullptr};
                auto* tag_result =
                    static_cast<Cc13SweepBNativeString*>(host.native_string_assign_0041e870(&tag_tmp, player_tag));
                assign_string(tag_text, tag_result);
                release_string(tag_tmp, host);
            }
            host.widget_vcall_34_set_visible(rank, at<std::uint8_t>(g(), 0x218C) == 0);
            host.frontend_set_player_rank_icon_frame_00517cb0(rank, at<std::uint8_t>(rec, 0x90));
            host.widget_vcall_34_set_visible(speak, 0);
            if (at<std::uint8_t>(g(), 0x218C) == 0 &&
                (at<std::uint8_t>(rec, 0x89) != 0 ||
                 (at<std::int32_t>(rec, 0x94) == 2 && host.player_record_is_talking_007f74b0(rec)))) {
                host.widget_vcall_34_set_visible(speak, 1);
                if (local != nullptr && local != rec &&
                    host.voice_mute_is_muted_one_way_00778740(
                        static_cast<std::uint8_t*>(g()) + 0x2080, at<std::uint32_t>(local, 0x80),
                        at<std::uint32_t>(local, 0x84), at<std::uint32_t>(rec, 0x80), at<std::uint32_t>(rec, 0x84))) {
                    host.widget_vcall_88(speak, 2, 0, one);
                } else if (host.player_record_is_talking_007f74b0(rec)) {
                    host.widget_vcall_88(speak, 1, 0, one);
                } else if (at<std::int32_t>(rec, 0x94) == 1) {
                    host.widget_vcall_88(speak, 0, 0, one);
                } else {
                    host.widget_vcall_34_set_visible(speak, 0);
                }
            }
            host.widget_vcall_34_set_visible(connection, at<std::uint8_t>(g(), 0x218C) == 0);
            float ping_sum = q0;
            if (at<std::uint32_t>(rec, 0x1C) != 0) {
                const float quality = host.game_peer_rows_get_row_quality_00786010(
                    static_cast<std::uint8_t*>(g()) + 0x1EF8, at<std::uint32_t>(rec, 0x1C));
                ping_sum = pc24(static_cast<double>(pc24(static_cast<double>(quality) + static_cast<double>(q0))) *
                                f64_bits(0x3FE0000000000000ull));  // 00D7A280
            }
            const float ping = pc24(static_cast<double>(ping_sum) + f64_bits(0x3EE4F8B580000000ull));  // 00D7A330
            cc13_sweepB3_session_row_update_ping_icons_00578580(connection, ping, host);
            const std::uint32_t mask_lo = at<std::uint32_t>(g(), 0x2060);
            const std::uint32_t mask_hi = at<std::uint32_t>(g(), 0x2064);
            if ((mask_lo & at<std::uint32_t>(rec, 0xD0)) == mask_lo && (mask_hi & at<std::uint32_t>(rec, 0xD4)) == mask_hi) {
                highlight(name);
                highlight(tag);
            } else {
                host.widget_vcall_50_colour(name, static_colour(0x00E19600, 0x00E195F0, one, 0.0f, 0.0f, one));
                host.widget_vcall_50_colour(tag, static_colour(0x00E19600, 0x00E195F0, one, 0.0f, 0.0f, one));
            }
        } else if (!joined && !flag9) {
            assign_cstr(text, cstr(0x00E088F8));
            host.widget_vcall_50_colour(name, static_colour(0x00E195EC, 0x00E195DC, one, one, one, one));
            host.widget_vcall_50_colour(tag, static_colour(0x00E195EC, 0x00E195DC, one, one, one, one));
        } else {
            if (at<std::uint8_t>(rec, 0xA) != 0) {
                assign_cstr(text, cstr(0x00E08940 + at<std::uint32_t>(rec, 0x2C) * 4));
                highlight(name);
                highlight(tag);
            } else {
                assign_cstr(text, cstr(0x00E088F8));
                host.widget_vcall_50_colour(name, static_colour(0x00E195EC, 0x00E195DC, one, one, one, one));
                host.widget_vcall_50_colour(tag, static_colour(0x00E195EC, 0x00E195DC, one, one, one, one));
            }
            host.widget_vcall_34_set_visible(rank, 0);
            host.widget_vcall_34_set_visible(droprate, 0);
            host.widget_vcall_34_set_visible(check, 0);
            host.widget_vcall_34_set_visible(speak, 0);
            host.widget_vcall_34_set_visible(connection, 0);
        }

        auto& selected = *static_cast<std::int32_t*>(host.image_global(0x00E08958));
        if (i == selected) {
            bool equal;
            if (*static_cast<std::uint32_t*>(host.image_global(0x00E195D4)) == 0) {
                equal = text.length == 0;
            } else {
                equal = text.length != 0 &&
                        host.crt_stricmp_00bf7fbf(cstr(0x00E195D8), static_cast<const char*>(text.data)) == 0;
            }
            if (equal) {
                assign_cstr(text, cstr(0x00E088FC));
            } else {
                selected = -1;
            }
        }
        host.gui_text_set_localised_source_00abaed0(name, &text, 1);
        host.gui_text_set_localised_source_00abaed0(tag, &tag_text, 1);
        if (at<std::uint32_t>(g(), 0x5FC) != 0 && i > 8) at<std::uint8_t>(row, 0x77) = 1;
        release_string(tag_text, host);
        release_string(text, host);
    }
}

// ---------------------------------------------------------------------------
// 007884D0 BSP_NetworkPacketStatistics_WriteCsvRow
// __fastcall(ECX = stats), RET. EH frame 00C8A8BD. Image facts (asm
// 007884D0..00789552):
// - p = [01090AB0] vtable +20h(buffer); +764930h = float(p[0] / p[1]) (FILD,
//   FILD, DE F9 FDIVP: counter / frequency, one rounding to float under the
//   main thread's 24-bit control; modelled as a double quotient, exact while
//   the counter stays below 2^53, then rounded to float).
// - For +7649A0h, then +7649A4h: "\n" (00CE4390) and ";" (00CE5698)
//   temporaries; a = 0043BDF0(+764930h); b = 0043BDF0(+76492Ch); c =
//   0043C130("Time;" 00D04328, b); d = c + ";"; e = d + a; f = e + "\n";
//   file vtable +5Ch(f, 0); released f, e, d, c, b, a, ";", "\n".
// - Rows: r = [+764994h] % 1001 (DIV); while r < [+764998h] % 1001
//   (unsigned, re-read after each row) and r < 1001 (signed): Row = stats +
//   4 + r*1DE8h; send entry k = Row + k*10h, receive entry k = Row + EF4h +
//   k*10h (k = 0..234; entry 234 holds the sums).
//   * Entry 0 of both directions reset (send: word +Eh = 0, +8h, +4h, +0h =
//     0, word +Ch = 9999 (270Fh); then receive: word +Ch = 9999, word +Eh =
//     0, +8h, +4h, +0h = 0); then for k = 0..233 the send sums Row+EA4h +=
//     entry+4h and Row+EA0h += entry+0h, the receive sums Row+1D98h and
//     Row+1D94h likewise.
//   * Each direction whose sum count (+EA0h of its base) is non-zero
//     (unsigned): ";" s1, ";" s2, ";" s3, "-" s4 and "" s5 (00CE3A0C, length
//     0); t1 = 004263B0(s5, word base+EF0h); t2 = t1 + s4; t3 =
//     004263B0(t2, word base+EF2h); t4 = t3 + s3; t5 = 00788380(t4,
//     base+EA4h); t6 = t5 + s2; t7 = 00788380(t6, base+EA4h / base+EA0h,
//     DIV); t8 = t7 + s1; file +5Ch(t8, 0); released t8..t1, s5, s4, s3,
//     s2, s1; then for k = 1..234 00788250(ECX = entry k, k, file) and the
//     entry reset (word +Eh = 0, +8h = 0, +4h = 0, word +Ch = 9999, +0h =
//     0); then "\n" written (+5Ch(&nl, 0)) and released.
// - Finally +764994h += [+76499Ch]; +76492Ch = +764930h; +764998h +=
//   [+76499Ch]; byte +7649A8h = 0.
// No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_packet_stats_write_csv_row_007884d0(void* stats, Cc13SweepB9Host& host) {
    alignas(8) std::uint8_t clock_buffer[16];
    const std::int64_t* clock =
        host.timer_vcall_20_read(*static_cast<void**>(host.image_global(0x01090AB0)), clock_buffer);
    at<float>(stats, 0x764930) = static_cast<float>(static_cast<double>(clock[0]) / static_cast<double>(clock[1]));

    auto literal = [&](Cc13SweepBNativeString& text, const char* source, std::uint32_t length) {
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
    };

    static const std::size_t kFileSlots[2] = {0x7649A0, 0x7649A4};
    for (std::size_t slot : kFileSlots) {
        Cc13SweepBNativeString newline = {0, nullptr};
        literal(newline, "\n", 1);  // 00CE4390
        Cc13SweepBNativeString semicolon = {0, nullptr};
        literal(semicolon, ";", 1);  // 00CE5698
        Cc13SweepBNativeString now = {0, nullptr};
        Cc13SweepBNativeString previous = {0, nullptr};
        Cc13SweepBNativeString time = {0, nullptr};
        Cc13SweepBNativeString d = {0, nullptr};
        Cc13SweepBNativeString e = {0, nullptr};
        Cc13SweepBNativeString f = {0, nullptr};
        void* a_out = host.native_string_from_float_0043bdf0(&now, at<float>(stats, 0x764930));
        void* b_out = host.native_string_from_float_0043bdf0(&previous, at<float>(stats, 0x76492C));
        void* c_out = host.native_string_prefixed_0043c130(&time, "Time;", b_out);  // 00D04328
        void* d_out = host.native_string_concat_004261a0(c_out, &d, &semicolon);
        void* e_out = host.native_string_concat_004261a0(d_out, &e, a_out);
        void* f_out = host.native_string_concat_004261a0(e_out, &f, &newline);
        host.log_file_vcall_5c_write(at<void*>(stats, slot), f_out, 0);
        release_string(f, host);
        release_string(e, host);
        release_string(d, host);
        release_string(time, host);
        release_string(previous, host);
        release_string(now, host);
        release_string(semicolon, host);
        release_string(newline, host);
    }

    auto write_direction = [&](std::uint8_t* base, std::size_t slot) {
        Cc13SweepBNativeString s1 = {0, nullptr};
        literal(s1, ";", 1);
        Cc13SweepBNativeString s2 = {0, nullptr};
        literal(s2, ";", 1);
        Cc13SweepBNativeString s3 = {0, nullptr};
        literal(s3, ";", 1);
        Cc13SweepBNativeString s4 = {0, nullptr};
        literal(s4, "-", 1);  // 00CEED40
        Cc13SweepBNativeString s5 = {0, nullptr};
        literal(s5, "", 0);  // 00CE3A0C
        Cc13SweepBNativeString t[8] = {};
        void* out = host.native_string_concat_int_004263b0(&s5, &t[0], at<std::uint16_t>(base, 0xEF0));
        out = host.native_string_concat_004261a0(out, &t[1], &s4);
        out = host.native_string_concat_int_004263b0(out, &t[2], at<std::uint16_t>(base, 0xEF2));
        out = host.native_string_concat_004261a0(out, &t[3], &s3);
        out = host.packet_stats_format_field_00788380(out, &t[4], at<std::uint32_t>(base, 0xEA4));
        out = host.native_string_concat_004261a0(out, &t[5], &s2);
        out = host.packet_stats_format_field_00788380(out, &t[6],
                                                      at<std::uint32_t>(base, 0xEA4) / at<std::uint32_t>(base, 0xEA0));
        out = host.native_string_concat_004261a0(out, &t[7], &s1);
        host.log_file_vcall_5c_write(at<void*>(stats, slot), out, 0);
        for (int k = 7; k >= 0; --k) release_string(t[k], host);
        release_string(s5, host);
        release_string(s4, host);
        release_string(s3, host);
        release_string(s2, host);
        release_string(s1, host);
        for (std::int32_t k = 1; k <= 0xEA; ++k) {
            std::uint8_t* entry = base + static_cast<std::size_t>(k) * 0x10;
            host.packet_stats_format_type_column_00788250(entry, k, at<void*>(stats, slot));
            at<std::uint16_t>(entry, 0xE) = 0;
            at<std::uint32_t>(entry, 0x8) = 0;
            at<std::uint32_t>(entry, 0x4) = 0;
            at<std::uint16_t>(entry, 0xC) = 9999;
            at<std::uint32_t>(entry, 0x0) = 0;
        }
        Cc13SweepBNativeString newline = {0, nullptr};
        literal(newline, "\n", 1);
        host.log_file_vcall_5c_write(at<void*>(stats, slot), &newline, 0);
        release_string(newline, host);
    };

    std::uint32_t r = at<std::uint32_t>(stats, 0x764994) % 1001;
    if (r < at<std::uint32_t>(stats, 0x764998) % 1001) {
        do {
            if (static_cast<std::int32_t>(r) >= 1001) break;
            std::uint8_t* const row = static_cast<std::uint8_t*>(stats) + 4 + static_cast<std::size_t>(r) * 0x1DE8;
            std::uint8_t* const send = row;
            std::uint8_t* const recv = row + 0xEF4;
            at<std::uint16_t>(send, 0xE) = 0;
            at<std::uint32_t>(send, 0x8) = 0;
            at<std::uint32_t>(send, 0x4) = 0;
            at<std::uint32_t>(send, 0x0) = 0;
            at<std::uint16_t>(send, 0xC) = 9999;
            at<std::uint16_t>(recv, 0xC) = 9999;
            at<std::uint16_t>(recv, 0xE) = 0;
            at<std::uint32_t>(recv, 0x8) = 0;
            at<std::uint32_t>(recv, 0x4) = 0;
            at<std::uint32_t>(recv, 0x0) = 0;
            for (std::size_t k = 0; k < 0xEA; ++k) {
                at<std::uint32_t>(send, 0xEA4) += at<std::uint32_t>(send, k * 0x10 + 4);
                at<std::uint32_t>(send, 0xEA0) += at<std::uint32_t>(send, k * 0x10);
                at<std::uint32_t>(recv, 0xEA4) += at<std::uint32_t>(recv, k * 0x10 + 4);
                at<std::uint32_t>(recv, 0xEA0) += at<std::uint32_t>(recv, k * 0x10);
            }
            if (at<std::uint32_t>(send, 0xEA0) > 0) write_direction(send, 0x7649A0);
            if (at<std::uint32_t>(recv, 0xEA0) > 0) write_direction(recv, 0x7649A4);
            ++r;
        } while (r < at<std::uint32_t>(stats, 0x764998) % 1001);
    }
    at<std::uint32_t>(stats, 0x764994) += at<std::uint32_t>(stats, 0x76499C);
    at<float>(stats, 0x76492C) = at<float>(stats, 0x764930);
    at<std::uint32_t>(stats, 0x764998) += at<std::uint32_t>(stats, 0x76499C);
    at<std::uint8_t>(stats, 0x7649A8) = 0;
}

// ---------------------------------------------------------------------------
// 00536A90 BSP_HudFormationScreen_UpdateOrderLabels
// __thiscall(ECX = screen; float dt), RET 4. EH frame 00C6C300. st = +170h.
// Image facts (asm 00536A90..00537AEB):
// - st == 128h: return. st not ECh / EFh: go to the fade part.
// - +169h != 0: skip to the confirm part. Otherwise six 004BEC00 calls read
//   a = im+30E4h, b = im+3114h, c = im+3564h, d = im+3594h, x = im+3744h,
//   y = im+3774h (im = [004BEC00()+4h]). |v| below is the SSE form v > 0 ?
//   v : -0.0f (00D7A208) - v (COMISS / JBE, so NaN takes the subtraction).
// - |y| + |x| (x87) > 0.8 (double 00CE3D40, FCOMIP / JBE): each arm sets
//   +140h = 0, +141h = 1, +142h = 0:
//   * 0 > y: +14Ch = 0; 0.1 (double 00D7A3A0) > +148h ? (+148h = -0.5f,
//     +144h = 0, +150h = -0.9f) : (+150h = 0, +148h = +148h - dt);
//   * else y > 0: +14Ch = 0; +148h > -0.01f (00D7A300) ? (+148h = 0.5f,
//     +144h = 0, +150h = 0.9f) : (+150h = 0, +148h += dt);
//   * else 0 > x: +150h = 0; +144h > -0.01f ? (+144h = 0.5f, +148h = 0,
//     +14Ch = -0.9f) : (+14Ch = 0, +144h += dt);
//   * else x > 0: +150h = 0; 0.01f (00D7A238) > +144h ? (+144h = -0.5f,
//     +148h = 0, +14Ch = 0.9f) : (+14Ch = 0, +144h -= dt).
// - Otherwise, when |d| + |c| > |b| + |a| (both x87 sums): +140h = 1, +141h
//   = +142h = 0; +14Ch = (c / 3.0 (double 00D7A2B0, DC F9)) + +14Ch; +150h =
//   (d / 3.0) + +150h (FDIVR); then when |c| > |d|: +14Ch > 0.55f
//   (00CED6C4) sets +14Ch = 0.6f, +150h = 0; -0.55f (00CED6C0) > +14Ch sets
//   +14Ch = -0.6f (00CED6BC), +150h = 0; else the same on +150h / +14Ch.
//   When |b| + |a| > |d| + |c|: +140h = +141h = 0, +142h = 1, +14Ch = a,
//   +150h = b. Otherwise (equal or NaN): S1 = 0042BE90(&c) + 0042BE90(&d)
//   (d first, kept as double), S2 = 0042BE90(&a) + 0042BE90(&b) (b
//   first); when S1 == S2 (FUCOMIP, LAHF / TEST 44h / JP): with +141h,
//   when 0.59f (00CED6B8) > 0042BE90(+150h) and > 0042BE90(+14Ch), +150h =
//   +14Ch = +144h = +148h = 0; then with +142h, +150h = +14Ch = 0.
// - Selection (00537121): 0.4 (double 00CE65D0) > |+150h| and > |+14Ch|:
//   +16Ch = 0. Else |+150h| > |+14Ch|: when -0.4 (double 00CED6B0) >
//   +150h and +E8h vcall +38h() (asked up to three times) is set, +16Ch =
//   1. Else +14Ch > 0.4: +DCh or +E4h +38h() gives 4; else -0.4 > +14Ch:
//   +D4h, +D8h or +E0h +38h() gives 3.
// - +FCh, +100h, +104h, +108h, +10Ch vcall +34h(0); +16Ch = 0 for (2, EFh),
//   (1, ECh), (2, ECh). Label: 3: +FCh shown, colour {1,1,1,1} (vcall +50h),
//   EFh "ingame.formation_diamond", ECh by [[+138h]+14h] +114h() +50h():
//   0 "ingame.orders_defensive" else "ingame.orders_aggressive"; 4: +100h,
//   ECh "ingame.formation_disband", EFh "ingame.formation_column"; 1 with
//   EFh: +104h, "ingame.formation_line"; 0: +FCh..+108h hidden, +10Ch shown,
//   "ingame.orders_cancel", +16Ch = 0 (00ABBE50(+F8h, text, 1) each).
// - Confirm (005375F1): key = [im+4h] + st*30h (one more 004BEC00 call);
//   unless (byte key+28h and key+24h > 0 (JA)) and when byte key+20h and
//   key+1Ch > 0: +168h = 0, +169h = 1, +164h = 0; ECh: 3 with +D8h or +D4h
//   +38h(): +13Ch = (+13Ch - 1) & 1, then for each member i < [+138h]+4F8h
//   (signed) 0070D060(+138h, i), when non-null its +114h() director gets
//   +54h(+13Ch); 4 with +DCh +38h(): the prompt 00531B00(00425D10(), 2,
//   "ingame.formation_areyousure" 00CED5E0, 1, 00535F30, 0, "", 0.0f, 0,
//   by-value "", 1), both named strings destroyed (0041DD20) afterwards.
//   EFh: (3, +E0h) shape 3, (1, +E8h) shape 1, (4, +E4h) shape 2:
//   0070EFD0(+138h, shape); 0077A080(stack message, +138h);
//   0077C880([+138h]+14h, message); 005347C0(message).
// - Fade (005378AD, only with +169h): +A8h vcall +54h(col); h = dt; with
//   +16Ch == 0: +164h = max(+164h, 1.0f) (COMISS / JBE) and h = dt * 2.0;
//   +164h += h; when +164h > 0.5f: col.a -= h * 2.0. Eight 004BEC00 calls
//   (results unused). When 0 >= col.a (COMISS / JC): +169h = +168h = 0,
//   st = 128h, 00533A30(screen), col.a = 1.0f. 00AA85B0(+A8h, col). +15Ch
//   += or -= h * 7.0 (double 00CED5D8) by +160h (DE E9: +15Ch - h*7);
//   +15Ch > 0.99 (double 00CED5D0) clears +160h, 0.01f > +15Ch sets it;
//   with +16Ch == 0: +15Ch = 0.5f, +160h = 1. The +FCh colour (+54h) with
//   alpha +15Ch goes to +FCh, +100h, +104h, +108h, +10Ch (+50h).
// x87 (rule 4c, main thread): each x87 operation rounds to float. No draw
// from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_formation_update_order_labels_00536a90(void* screen, float dt, Cc13SweepB9Host& host) {
    const std::int32_t state = at<std::int32_t>(screen, 0x170);
    if (state == 0x128) return;
    const float zero = 0.0f;
    const float one = f32_bits(0x3F800000u);
    auto sse_abs = [](float v) { return v > 0.0f ? v : f32_bits(0x80000000u) - v; };  // 00D7A208
    auto input_float = [&](std::size_t offset) {
        return at<float>(at<void*>(host.input_manager_singleton_004bec00(), 0x4), offset);
    };
    auto shown = [&](std::size_t slot) { return host.widget_vcall_38_is_visible(at<void*>(screen, slot)); };
    auto set_flags = [&](std::uint8_t f140, std::uint8_t f141, std::uint8_t f142) {
        at<std::uint8_t>(screen, 0x140) = f140;
        at<std::uint8_t>(screen, 0x141) = f141;
        at<std::uint8_t>(screen, 0x142) = f142;
    };
    float& p144 = at<float>(screen, 0x144);
    float& p148 = at<float>(screen, 0x148);
    float& p14c = at<float>(screen, 0x14C);
    float& p150 = at<float>(screen, 0x150);

    if (state == 0xEC || state == 0xEF) {
        if (at<std::uint8_t>(screen, 0x169) == 0) {
            float a = input_float(0x30E4);
            float b = input_float(0x3114);
            float c = input_float(0x3564);
            float d = input_float(0x3594);
            const float x = input_float(0x3744);
            const float y = input_float(0x3774);
            if (static_cast<double>(pc24(static_cast<double>(sse_abs(y)) + sse_abs(x))) >
                f64_bits(0x3FE999999A000000ull)) {  // 00CE3D40
                if (zero > y) {
                    const float v = p148;
                    set_flags(0, 1, 0);
                    p14c = zero;
                    if (f64_bits(0x3FB999999A000000ull) > static_cast<double>(v)) {  // 00D7A3A0
                        p148 = f32_bits(0xBF000000u);  // 00CE69D0
                        p144 = zero;
                        p150 = f32_bits(0xBF666666u);  // 00CED6C8
                    } else {
                        p150 = zero;
                        p148 = pc24(static_cast<double>(v) - dt);
                    }
                } else if (y > zero) {
                    const float v = p148;
                    set_flags(0, 1, 0);
                    p14c = zero;
                    if (v > f32_bits(0xBC23D70Au)) {  // 00D7A300
                        p148 = f32_bits(0x3F000000u);  // 00CE3800
                        p144 = zero;
                        p150 = f32_bits(0x3F666666u);  // 00CE3860
                    } else {
                        p150 = zero;
                        p148 = pc24(static_cast<double>(v) + dt);
                    }
                } else if (zero > x) {
                    const float v = p144;
                    set_flags(0, 1, 0);
                    p150 = zero;
                    if (v > f32_bits(0xBC23D70Au)) {
                        p144 = f32_bits(0x3F000000u);
                        p148 = zero;
                        p14c = f32_bits(0xBF666666u);
                    } else {
                        p14c = zero;
                        p144 = pc24(static_cast<double>(v) + dt);
                    }
                } else if (x > zero) {
                    const float v = p144;
                    set_flags(0, 1, 0);
                    p150 = zero;
                    if (f32_bits(0x3C23D70Au) > v) {  // 00D7A238
                        p144 = f32_bits(0xBF000000u);
                        p148 = zero;
                        p14c = f32_bits(0x3F666666u);
                    } else {
                        p14c = zero;
                        p144 = pc24(static_cast<double>(v) - dt);
                    }
                }
            } else {
                const float abs_c = sse_abs(c);
                const float abs_d = sse_abs(d);
                const float abs_a = sse_abs(a);
                const float abs_b = sse_abs(b);
                const float right = pc24(static_cast<double>(abs_d) + abs_c);
                const float left = pc24(static_cast<double>(abs_b) + abs_a);
                if (right > left) {
                    const double three = f64_bits(0x4008000000000000ull);  // 00D7A2B0
                    set_flags(1, 0, 0);
                    const float c_step = pc24(static_cast<double>(c) / three);
                    const float new_14c = pc24(static_cast<double>(c_step) + p14c);
                    p14c = new_14c;
                    const float d_step = pc24(static_cast<double>(d) / three);
                    const float new_150 = pc24(static_cast<double>(d_step) + p150);
                    p150 = new_150;
                    const float hi = f32_bits(0x3F0CCCCDu);   // 00CED6C4
                    const float lo = f32_bits(0xBF0CCCCDu);   // 00CED6C0
                    const float snap = f32_bits(0x3F19999Au);  // 00CE3D30
                    const float snap_neg = f32_bits(0xBF19999Au);  // 00CED6BC
                    if (sse_abs(c) > sse_abs(d)) {
                        if (new_14c > hi) {
                            p14c = snap;
                            p150 = zero;
                        }
                        if (lo > p14c) {
                            p14c = snap_neg;
                            p150 = zero;
                        }
                    } else {
                        if (new_150 > hi) {
                            p150 = snap;
                            p14c = zero;
                        }
                        if (lo > p150) {
                            p150 = snap_neg;
                            p14c = zero;
                        }
                    }
                } else {
                    const float sum_cd = pc24(static_cast<double>(sse_abs(d)) + sse_abs(c));
                    const float sum_ab = pc24(static_cast<double>(sse_abs(b)) + sse_abs(a));
                    if (sum_ab > sum_cd) {
                        set_flags(0, 0, 1);
                        p14c = a;
                        p150 = b;
                    } else {
                        const float abs_d_ref = host.math_abs_float_by_ref_0042be90(&d);
                        const double s1 =
                            pc24(static_cast<double>(host.math_abs_float_by_ref_0042be90(&c)) + abs_d_ref);
                        const float abs_b_ref = host.math_abs_float_by_ref_0042be90(&b);
                        const float s2 =
                            pc24(static_cast<double>(host.math_abs_float_by_ref_0042be90(&a)) + abs_b_ref);
                        if (s1 == static_cast<double>(s2)) {
                            const float dead = f32_bits(0x3F170A3Du);  // 00CED6B8
                            if (at<std::uint8_t>(screen, 0x141) != 0 &&
                                dead > host.math_abs_float_by_ref_0042be90(&p150) &&
                                dead > host.math_abs_float_by_ref_0042be90(&p14c)) {
                                p150 = zero;
                                p14c = zero;
                                p144 = zero;
                                p148 = zero;
                            }
                            if (at<std::uint8_t>(screen, 0x142) != 0) {
                                p150 = zero;
                                p14c = zero;
                            }
                        }
                    }
                }
            }

            // Selection.
            const double near = f64_bits(0x3FD999999A000000ull);   // 00CE65D0
            const double far = f64_bits(0xBFD999999A000000ull);    // 00CED6B0
            if (near > static_cast<double>(sse_abs(p150)) && near > static_cast<double>(sse_abs(p14c))) {
                at<std::int32_t>(screen, 0x16C) = 0;
            } else if (sse_abs(p150) > sse_abs(p14c)) {
                if (far > static_cast<double>(p150) && (shown(0xE8) || shown(0xE8) || shown(0xE8))) {
                    at<std::int32_t>(screen, 0x16C) = 1;
                }
            } else {
                const float p = p14c;
                if (static_cast<double>(p) > near) {
                    if (shown(0xDC) || shown(0xE4)) at<std::int32_t>(screen, 0x16C) = 4;
                } else if (far > static_cast<double>(p)) {
                    if (shown(0xD4) || shown(0xD8) || shown(0xE0)) at<std::int32_t>(screen, 0x16C) = 3;
                }
            }

            for (std::size_t slot = 0xFC; slot <= 0x10C; slot += 4) host.widget_vcall_34_set_visible(at<void*>(screen, slot), 0);
            std::int32_t& entry = at<std::int32_t>(screen, 0x16C);
            const std::int32_t st = at<std::int32_t>(screen, 0x170);
            if ((entry == 2 && st == 0xEF) || (entry == 1 && st == 0xEC) || (entry == 2 && st == 0xEC)) entry = 0;
            auto show_label = [&](std::size_t slot, std::int32_t show) {
                host.widget_vcall_34_set_visible(at<void*>(screen, slot), show);
                const float white[4] = {one, one, one, one};  // 00D7A24C
                host.widget_vcall_50_colour(at<void*>(screen, slot), white);
            };
            auto label = [&](const char* text) {
                host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, 0xF8), text, 1);
            };
            const std::int32_t mode = entry;
            if (mode == 3) {
                show_label(0xFC, 1);
                const std::int32_t st2 = at<std::int32_t>(screen, 0x170);
                if (st2 == 0xEF) {
                    label("ingame.formation_diamond");  // 00CED694
                } else if (st2 == 0xEC) {
                    void* director = host.unit_vcall_114_director(at<void*>(at<void*>(screen, 0x138), 0x14));
                    label(host.director_vcall_50_stance(director) == 0 ? "ingame.orders_defensive"     // 00CED67C
                                                                         : "ingame.orders_aggressive");  // 00CED660
                }
            } else if (mode == 4) {
                const std::int32_t st2 = at<std::int32_t>(screen, 0x170);
                if (st2 == 0xEC) {
                    show_label(0x100, 1);
                    label("ingame.formation_disband");  // 00CED644
                } else if (st2 == 0xEF) {
                    show_label(0x100, 1);
                    label("ingame.formation_column");  // 00CED62C
                }
            } else if (mode == 1) {
                if (at<std::int32_t>(screen, 0x170) == 0xEF) {
                    show_label(0x104, 1);
                    label("ingame.formation_line");  // 00CED614
                }
            } else if (mode == 0) {  // mode 2: no label
                for (std::size_t slot = 0xFC; slot <= 0x108; slot += 4) {
                    host.widget_vcall_34_set_visible(at<void*>(screen, slot), 0);
                }
                show_label(0x10C, 1);
                label("ingame.orders_cancel");  // 00CED5FC
                entry = 0;
            }
        }

        // Confirm.
        const std::int32_t st = at<std::int32_t>(screen, 0x170);
        const std::uint8_t* key = static_cast<const std::uint8_t*>(
                                      at<void*>(host.input_manager_singleton_004bec00(), 0x4)) +
                                  static_cast<std::size_t>(st) * 0x30;
        const bool released = key[0x28] != 0 && *reinterpret_cast<const float*>(key + 0x24) > zero;
        if (!released && key[0x20] != 0 && *reinterpret_cast<const float*>(key + 0x1C) > zero) {
            const std::int32_t st2 = at<std::int32_t>(screen, 0x170);
            at<std::uint8_t>(screen, 0x168) = 0;
            at<std::uint8_t>(screen, 0x169) = 1;
            at<float>(screen, 0x164) = zero;
            // +16Ch is re-read at every comparison, as in the image.
            auto entry_is = [&](std::int32_t value) { return at<std::int32_t>(screen, 0x16C) == value; };
            if (st2 == 0xEC) {
                if (entry_is(3)) {
                    if (shown(0xD8) || shown(0xD4)) {
                        at<std::uint32_t>(screen, 0x13C) = (at<std::uint32_t>(screen, 0x13C) - 1) & 1u;
                        const std::int32_t members = at<std::int32_t>(at<void*>(screen, 0x138), 0x4F8);
                        for (std::int32_t i = 0; i < members; ++i) {
                            void* unit = host.unit_group_member_at_0070d060(at<void*>(screen, 0x138), i);
                            if (unit != nullptr) {
                                host.director_vcall_54_set_stance(host.unit_vcall_114_director(unit),
                                                                  at<std::int32_t>(screen, 0x13C));
                            }
                        }
                    }
                } else if (entry_is(4)) {
                    if (shown(0xDC)) {
                        Cc13SweepBNativeString title;
                        host.native_string_assign_0041e870(&title, "");  // 00CE3A0C
                        Cc13SweepBNativeString message;
                        host.native_string_assign_0041e870(&message, "ingame.formation_areyousure");  // 00CED5E0
                        Cc13SweepBNativeString by_value;
                        host.native_string_assign_0041e870(&by_value, "");
                        void* menu = host.menu_command_screen_00425d10();
                        host.menu_prompt_raise_00531b00(menu, 2, &message, 1, 0x00535F30u, 0, &title, 0.0f, 0, by_value,
                                                        1);
                        host.native_string_destroy_storage_0041dd20(&message);
                        host.native_string_destroy_storage_0041dd20(&title);
                    }
                }
            } else if (st2 == 0xEF) {
                std::int32_t shape = -1;
                if (entry_is(3) && shown(0xE0)) {
                    shape = 3;
                } else if (entry_is(1) && shown(0xE8)) {
                    shape = 1;
                } else if (entry_is(4) && shown(0xE4)) {
                    shape = 2;
                }
                if (shape >= 0) {
                    alignas(8) std::uint8_t message[0x30];  // [ESP+60h]
                    host.unit_group_apply_formation_shape_0070efd0(at<void*>(screen, 0x138), shape);
                    void* built = host.session_message_formation_set_construct_0077a080(message, at<void*>(screen, 0x138));
                    host.unit_group_broadcast_formation_0077c880(at<void*>(at<void*>(screen, 0x138), 0x14), built);
                    host.session_message_formation_destruct_005347c0(message);
                }
            }
        }
    }

    // Fade.
    if (at<std::uint8_t>(screen, 0x169) == 0) return;
    float colour[4];
    host.widget_vcall_54_colour(at<void*>(screen, 0xA8), colour);
    const double two = f64_bits(0x4000000000000000ull);  // 00D7A308
    float step = dt;
    if (at<std::int32_t>(screen, 0x16C) == 0) {
        if (one > at<float>(screen, 0x164)) at<float>(screen, 0x164) = one;
        step = pc24(static_cast<double>(dt) * two);
    }
    const float open = pc24(static_cast<double>(at<float>(screen, 0x164)) + step);
    at<float>(screen, 0x164) = open;
    if (open > f32_bits(0x3F000000u)) {  // 00CE3800
        colour[3] = pc24(static_cast<double>(colour[3]) - pc24(static_cast<double>(step) * two));
    }
    for (int k = 0; k < 8; ++k) host.input_manager_singleton_004bec00();
    if (zero >= colour[3]) {
        at<std::uint8_t>(screen, 0x169) = 0;
        at<std::uint8_t>(screen, 0x168) = 0;
        at<std::int32_t>(screen, 0x170) = 0x128;
        host.formation_screen_hide_all_widgets_00533a30(screen);
        colour[3] = one;
    }
    host.gui_widget_set_colour_00aa85b0(at<void*>(screen, 0xA8), colour);
    const double seven = f64_bits(0x401C000000000000ull);  // 00CED5D8
    float& pulse = at<float>(screen, 0x15C);
    if (at<std::uint8_t>(screen, 0x160) != 0) {
        pulse = pc24(static_cast<double>(pc24(static_cast<double>(step) * seven)) + pulse);
    } else {
        pulse = pc24(static_cast<double>(pulse) - pc24(static_cast<double>(step) * seven));
    }
    const float level = pulse;
    if (static_cast<double>(level) > f64_bits(0x3FEFAE1480000000ull)) at<std::uint8_t>(screen, 0x160) = 0;  // 00CED5D0
    if (f32_bits(0x3C23D70Au) > level) at<std::uint8_t>(screen, 0x160) = 1;  // 00D7A238
    if (at<std::int32_t>(screen, 0x16C) == 0) {
        pulse = f32_bits(0x3F000000u);
        at<std::uint8_t>(screen, 0x160) = 1;
    }
    float scratch[4];
    const float* source = host.widget_vcall_54_colour(at<void*>(screen, 0xFC), scratch);
    float highlight[4] = {source[0], source[1], source[2], source[3]};
    highlight[3] = pulse;
    for (std::size_t slot = 0xFC; slot <= 0x10C; slot += 4) host.widget_vcall_50_colour(at<void*>(screen, slot), highlight);
}

// ---------------------------------------------------------------------------
// 005B3470 BSP_HudMapScreen_BuildPlayerList
// __fastcall(ECX = screen), RET; vtable 00CF0118 slot 13. EH frame 00C72070.
// Image facts (asm 005B3470..005B45B3); gui = [00E198C4]:
// - 00A7D120([00F8BBD8], 1). When [00E188D8] and gui+58h are non-null,
//   bytes [00E188D8]+5Eh and +5Dh are clear and the local player record's
//   byte +19h is clear: 005BCA30(gui+58h, 0).
// - 00AA7E00(parent, literal, 1) with the result stored before the name's
//   release: All_Group under +8Ch to +94h; then under +94h, each hidden
//   (vcall +34h(0)) after the release: PlayerNameTemplate_Text +98h,
//   PlayerTalkingTemplate_Icon +9Ch, radio_Icon +A0h, actunit_Icon +A4h,
//   PupTemplate_Icon +A8h, unit_name_Text +C4h, WarningTemplate_Icon +B0h,
//   HealthTemplate_Icon +B4h, JapCaptureTemplate_Icon +B8h,
//   USCaptureTemplate_Icon +BCh, HealthTemplate2_Icon +C0h,
//   distance_A_Icon +C8h, distance_B_Icon +CCh, map_corner_Icon +D0h.
// - The vector +D4h (begin +D8h, end +DCh): 00BF6713 when begin > end (two
//   checks, the end read first); 005A0AC0 erase of [begin, end);
//   005A5A80(11, 0); "popup_FrameBox" into element 0 (00BF6713 unless the
//   begin is non-null and the size non-zero, the element address read
//   before the find), then element 0 hidden after the same check.
// - Under +94h, hidden: select_Icon +E4h, target_Icon +E8h, mission_HL_Icon
//   +ECh, filter_panel_Icon +100h, filter_item_Icon +104h,
//   filter_panel2_Icon +120h, filter_Text +124h; under +90h map_class_Icon
//   +ACh, hidden.
// - When 00644230(gui+40h) is non-null: +54Ch = its vtable +C4h(buf)[0] and
//   +550h = [2] (00644230 called again for each). +554h = +54Ch, +558h =
//   +550h; 00AA1FA0(004C12B0(), 0.5f, 0.5f) (the two floats pushed by
//   value). Byte [00E188A8]+19C4h = 1; +520h, +530h, +52Ch, +538h, +534h,
//   byte +528h, +524h = 0. 005B2D70(screen) (sweepB5), 005293B0(gui+C8h),
//   0068E420(004E1CA0()), 0054E610(gui+A8h).
// - "MAP1STUSE": when 0068E600 says it was shown, 00690FD0("MAPPERMANENT",
//   0), else 00690FD0("MAP1STUSE", 0) (0041E870 temporaries, inline
//   release).
// - +56Ch = 00AD56D0([00F8C210]); when non-null, for each node (+0Ch first,
//   +3Ch next) whose 00B6D800 name is non-null and _stricmp-equal to
//   "Terrain Visibility Group": every child (+34h first, +3Ch next) whose
//   name differs from "BushGroup" (inline native compare: empty name ->
//   equal only to an empty literal, else _stricmp) gets 00B6DA70(child,
//   0.0f, 0); the node is appended to the pointer array +570h (0059E5E0
//   with max(2*capacity, 1) when full). Then 00B6D890(element, +0Ch) for
//   each element and 0059FCE0(+570h, 0).
// - +12Ch, +58Ch, +590h, +594h = 0.0f; byte +598h = 1; +59Ch = 0; +60Ch =
//   1.0f; +610h = 0.
// No x87 arithmetic (copies only), no draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB9_hud_map_build_player_list_005b3470(void* screen, Cc13SweepB9Host& host) {
    host.sound_manager_push_pop_speech_duck_00a7d120(*static_cast<void**>(host.image_global(0x00F8BBD8)), 1);
    void* const session = host.global_00e188d8();
    if (session != nullptr) {
        void* gui = in_game_gui(host);
        if (at<void*>(gui, 0x58) != nullptr && at<std::uint8_t>(session, 0x5E) == 0 &&
            at<std::uint8_t>(session, 0x5D) == 0) {
            void* game = host.global_00e188a8();
            void* local = at<void*>(game, 0x18CC + at<std::uint32_t>(game, 0x18EC) * 4);
            if (at<std::uint8_t>(local, 0x19) == 0) host.hud_panel_set_visible3_005bca30(at<void*>(gui, 0x58), 0);
        }
    }

    auto bind = [&](std::size_t parent, const char* name, std::uint32_t length, std::size_t slot) {
        with_c_string(name, length, host, [&](Cc13SweepBNativeString* text) {
            at<void*>(screen, slot) = host.widget_find_child_00aa7e00(at<void*>(screen, parent), text, 1);
        });
    };
    auto bind_hidden = [&](std::size_t parent, const char* name, std::uint32_t length, std::size_t slot) {
        bind(parent, name, length, slot);
        host.widget_vcall_34_set_visible(at<void*>(screen, slot), 0);
    };

    bind(0x8C, "All_Group", 9, 0x94);  // 00CF0D38
    struct Child {
        const char* name;
        std::uint32_t length;
        std::size_t slot;
    };
    static const Child kTemplates[] = {
        {"PlayerNameTemplate_Text", 0x17, 0x98},     {"PlayerTalkingTemplate_Icon", 0x1A, 0x9C},  // 00CF0D20 00CF0D04
        {"radio_Icon", 0xA, 0xA0},                   {"actunit_Icon", 0xC, 0xA4},                  // 00CF0CF8 00CF0CE8
        {"PupTemplate_Icon", 0x10, 0xA8},            {"unit_name_Text", 0xE, 0xC4},                // 00CF0CD4 00CED824
        {"WarningTemplate_Icon", 0x14, 0xB0},        {"HealthTemplate_Icon", 0x13, 0xB4},          // 00CF0CBC 00CF0CA8
        {"JapCaptureTemplate_Icon", 0x17, 0xB8},     {"USCaptureTemplate_Icon", 0x16, 0xBC},       // 00CF0C90 00CF0C78
        {"HealthTemplate2_Icon", 0x14, 0xC0},        {"distance_A_Icon", 0xF, 0xC8},               // 00CF0C60 00CF0C50
        {"distance_B_Icon", 0xF, 0xCC},              {"map_corner_Icon", 0xF, 0xD0},               // 00CF0C40 00CF0C30
    };
    for (const Child& child : kTemplates) bind_hidden(0x94, child.name, child.length, child.slot);

    // The pointer vector +D4h: cleared and resized to 11 entries.
    std::uint8_t* const popups = static_cast<std::uint8_t*>(screen) + 0xD4;
    void* const old_end = at<void*>(popups, 0x8);
    if (at<std::uint8_t*>(popups, 0x4) > static_cast<std::uint8_t*>(old_end)) host.crt_invalid_parameter_noinfo_00bf6713();
    void* const old_begin = at<void*>(popups, 0x4);
    if (static_cast<std::uint8_t*>(old_begin) > at<std::uint8_t*>(popups, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
    void* erased[2];
    host.pointer_vector_erase_range_005a0ac0(popups, erased, popups, old_begin, popups, old_end);
    host.pointer_vector_resize_005a5a80(popups, 0xB, 0);
    auto check_first = [&]() {
        void** begin = at<void**>(popups, 0x4);
        if (begin == nullptr ||
            (static_cast<std::int32_t>(at<std::uintptr_t>(popups, 0x8) - reinterpret_cast<std::uintptr_t>(begin)) >> 2) == 0) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    with_c_string("popup_FrameBox", 0xE, host, [&](Cc13SweepBNativeString* text) {  // 00CF0C20
        check_first();
        void** element = at<void**>(popups, 0x4);
        *element = host.widget_find_child_00aa7e00(at<void*>(screen, 0x94), text, 1);
    });
    check_first();
    host.widget_vcall_34_set_visible(*at<void**>(popups, 0x4), 0);

    static const Child kIcons[] = {
        {"select_Icon", 0xB, 0xE4},         {"target_Icon", 0xB, 0xE8},          // 00CF0C14 00CF0C08
        {"mission_HL_Icon", 0xF, 0xEC},     {"filter_panel_Icon", 0x11, 0x100},  // 00CF0BF8 00CF0BE4
        {"filter_item_Icon", 0x10, 0x104},  {"filter_panel2_Icon", 0x12, 0x120}, // 00CF0BD0 00CF0BBC
        {"filter_Text", 0xB, 0x124},        // 00CEE938
    };
    for (const Child& child : kIcons) bind_hidden(0x94, child.name, child.length, child.slot);
    bind_hidden(0x90, "map_class_Icon", 0xE, 0xAC);  // 00CED798

    float position[3];
    if (host.trivial_body_00644230(at<void*>(in_game_gui(host), 0x40)) != nullptr) {
        at<float>(screen, 0x54C) = host.unit_vcall_c4_position(
            host.trivial_body_00644230(at<void*>(in_game_gui(host), 0x40)), position)[0];
        at<float>(screen, 0x550) = host.unit_vcall_c4_position(
            host.trivial_body_00644230(at<void*>(in_game_gui(host), 0x40)), position)[2];
    }
    at<float>(screen, 0x554) = at<float>(screen, 0x54C);
    at<float>(screen, 0x558) = at<float>(screen, 0x550);
    host.gui_manager_set_cursor_00aa1fa0(host.gui_manager_get_004c12b0(), f32_bits(0x3F000000u),
                                         f32_bits(0x3F000000u));  // 00CE3800
    at<std::uint8_t>(host.global_00e188a8(), 0x19C4) = 1;
    at<std::uint32_t>(screen, 0x520) = 0;
    at<std::uint32_t>(screen, 0x530) = 0;
    at<std::uint32_t>(screen, 0x52C) = 0;
    at<std::uint32_t>(screen, 0x538) = 0;
    at<std::uint32_t>(screen, 0x534) = 0;
    at<std::uint8_t>(screen, 0x528) = 0;
    at<std::uint32_t>(screen, 0x524) = 0;
    cc13_sweepB5_hud_map_load_background_005b2d70(screen, host);
    host.hud_counters_hide_under_screen54_005293b0(at<void*>(in_game_gui(host), 0xC8));
    host.award_tracker_finish_active_hint_0068e420(host.award_tracker_get_004e1ca0());
    host.hud_hints_dismiss_0054e610(at<void*>(in_game_gui(host), 0xA8));

    bool shown = false;
    with_c_string("MAP1STUSE", 9, host, [&](Cc13SweepBNativeString* text) {  // 00CF0BB0
        shown = host.award_tracker_has_hint_been_shown_0068e600(host.award_tracker_get_004e1ca0(), text);
    });
    {
        Cc13SweepBNativeString hint = {0, nullptr};
        host.native_string_assign_0041e870(&hint, shown ? "MAPPERMANENT" : "MAP1STUSE");  // 00CEDA00 / 00CF0BB0
        host.award_tracker_trigger_hint_00690fd0(host.award_tracker_get_004e1ca0(), &hint, 0);
        release_string(hint, host);
    }

    void* const root = host.trivial_body_00ad56d0(*static_cast<void**>(host.image_global(0x00F8C210)));
    at<void*>(screen, 0x56C) = root;
    if (root == nullptr) {
        // The image skips the walk and the array work (005B43C8).
    } else {
        std::uint8_t* const array = static_cast<std::uint8_t*>(screen) + 0x570;
        for (void* node = at<void*>(root, 0xC); node != nullptr; node = at<void*>(node, 0x3C)) {
            const auto* name = static_cast<const Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(node));
            if (name->data == nullptr ||
                host.crt_stricmp_00bf7fbf(static_cast<const char*>(name->data), "Terrain Visibility Group") != 0) {  // 00CF00D8
                continue;
            }
            for (void* child = at<void*>(node, 0x34); child != nullptr; child = at<void*>(child, 0x3C)) {
                Cc13SweepBNativeString bush = {0, nullptr};
                host.native_string_resize_0041dd40(&bush, 9, 1);
                if (bush.data != nullptr) host.crt_memcpy_00bf7680(bush.data, "BushGroup", bush.length + 1);  // 00CF00CC
                const auto* child_name =
                    static_cast<const Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(child));
                std::int32_t order;
                if (child_name->length == 0) {
                    order = bush.length != 0 ? -1 : 0;
                } else if (bush.length == 0) {
                    order = 1;
                } else {
                    order = host.crt_stricmp_00bf7fbf(static_cast<const char*>(child_name->data),
                                                      static_cast<const char*>(bush.data));
                }
                const bool differs = order != 0;
                release_string(bush, host);
                if (differs) host.scene_node_set_visibility_factor_00b6da70(child, 0.0f, 0);
            }
            if (at<std::int32_t>(array, 0x4) == at<std::int32_t>(array, 0x8)) {
                std::int32_t capacity = at<std::int32_t>(array, 0x8) * 2;
                if (!(capacity > 1)) capacity = 1;
                host.pointer_array_reserve_0059e5e0(array, capacity);
            }
            void** slot = at<void**>(array, 0x0) + at<std::int32_t>(array, 0x4);
            if (slot != nullptr) *slot = node;
            at<std::int32_t>(array, 0x4) += 1;
        }
        for (std::int32_t i = 0; i < at<std::int32_t>(array, 0x4); ++i) {
            host.node_propagate_root_00b6d890(at<void**>(array, 0x0)[i], at<void*>(screen, 0xC));
        }
        host.pointer_array_resize_0059fce0(array, 0);
    }
    at<float>(screen, 0x12C) = 0.0f;
    at<float>(screen, 0x58C) = 0.0f;
    at<float>(screen, 0x590) = 0.0f;
    at<float>(screen, 0x594) = 0.0f;
    at<std::uint8_t>(screen, 0x598) = 1;
    at<std::uint32_t>(screen, 0x59C) = 0;
    at<float>(screen, 0x60C) = f32_bits(0x3F800000u);  // 00D7A24C
    at<std::uint32_t>(screen, 0x610) = 0;
}

}  // namespace bsp
