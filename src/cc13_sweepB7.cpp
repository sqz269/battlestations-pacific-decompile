// cc13_sweepB7: seventh pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB7, docs/CC13_SWEEPB7.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB..B6 and cc13_pkt642): float expressions follow the
// image's operation order with a float store wherever the image stores to a
// float slot; intermediate x87 values are kept in double, and double
// constants are applied in double. EH frames are noted, not modelled.

#include "bsp/cc13_sweepB7.hpp"

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

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepB7Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined temporary native string of a literal: {0, 0}, 0041DD40(length,
// 1), memcpy of length + 1 bytes when data is non-null, the use, then the
// release (the length re-read after the use).
template <class Use>
void with_c_string(const char* source, std::uint32_t length, Cc13SweepB7Host& host, Use use) {
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
    use(&text);
    release_string(text, host);
}

// 00E198C4, the in-game GUI.
void* in_game_gui(Cc13SweepB7Host& host) {
    return host.global_00e198c4();
}

}  // namespace

// ---------------------------------------------------------------------------
// 006191A0 BSP_HudPowerupManagerScreen_RebuildSecondSlots
// __fastcall(ECX = screen), RET. Image facts (asm 006191A0..00619ADF):
// - +1F9h = 1; +A0h and +A4h vcall +34h(0).
// - +1F8h == 0 (006191F6): +214h/+218h/+210h save +74h/+94h/+64h; +88h hidden
//   (the 0 was pushed at 006191EF before the branch); the columns shift up
//   (+64h<-+60h<-+5Ch<-+58h, same for +74h..+68h and +94h..+88h) and the new
//   slot goes into +58h/+68h/+88h. Otherwise (0061964C) +218h/+214h/+210h save
//   +88h/+68h/+58h; +94h hidden; the columns shift down and the new slot goes
//   into +64h/+74h/+94h.
// - The slot (both arms identical apart from the slots): 00AAB4C0 on the
//   template [[00E198C4]+BCh]+138h with parent 0, shown; "icon_L_Icon"
//   (00CF4DDC) under it is the icon; "number_Text" (00CF4DD0) under the icon
//   is hidden; icon +88h(3, 0, 1.0f); for i from (short)icon +98h() while
//   i < [[00E198C4]+C0h]+1E0h + start (unsigned JC, bound re-read each pass)
//   the map +1D8h is subscripted twice with key i (006170F0) and 00AB64C0
//   adds state (value+D0h, value+D4h) to the icon; the checked index +1F4h
//   into the vector +188h/+18Ch (00BF6713 on failure, then the read anyway)
//   gives a record whose +4h key looks up a word in the map +1CCh (00617870),
//   and icon +88h(word, 0, 1.0f); "circle_small_01_Group" (00CF4DA8) and
//   "circle_small_02_Group" (00CF4D90) under the clone are hidden;
//   "circle_HL_Icon" (00CF4DC0) under the clone is stored and shown.
// - Positions (vcall +44h, one float): up arm icon gets -0.0f (00D7A208) -
//   bc+2A4h (SUBSS) and the clone bc+2A4h; down arm icon gets
//   bc+2A8h * 5.0 (double 00D7A370) - bc+2A4h, the clone the same with +,
//   each stored as a float; bc = [00E198C4]+BCh, re-read every time. Main
//   thread, 24-bit PC: the product and the sum each round to float.
// - 0065E870(ECX = bc, new clone, 0.0f); +1FCh = 1.0f (00D7A24C).
// The name temporaries use 0041DD40 / memcpy / pool release as everywhere;
// the EH states 0..9 track them.
// ---------------------------------------------------------------------------
void cc13_sweepB7_powerup_screen_rebuild_second_slots_006191a0(void* screen, Cc13SweepB7Host& host) {
    at<std::uint8_t>(screen, 0x1F9) = 1;
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xA0), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xA4), 0);

    const bool up = at<std::uint8_t>(screen, 0x1F8) == 0;
    std::size_t clone_slot;
    std::size_t icon_slot;
    std::size_t highlight_slot;
    if (up) {
        at<std::uint32_t>(screen, 0x214) = at<std::uint32_t>(screen, 0x74);
        at<std::uint32_t>(screen, 0x218) = at<std::uint32_t>(screen, 0x94);
        at<std::uint32_t>(screen, 0x210) = at<std::uint32_t>(screen, 0x64);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x88), 0);
        at<std::uint32_t>(screen, 0x64) = at<std::uint32_t>(screen, 0x60);
        at<std::uint32_t>(screen, 0x94) = at<std::uint32_t>(screen, 0x90);
        at<std::uint32_t>(screen, 0x74) = at<std::uint32_t>(screen, 0x70);
        at<std::uint32_t>(screen, 0x60) = at<std::uint32_t>(screen, 0x5C);
        at<std::uint32_t>(screen, 0x90) = at<std::uint32_t>(screen, 0x8C);
        at<std::uint32_t>(screen, 0x70) = at<std::uint32_t>(screen, 0x6C);
        at<std::uint32_t>(screen, 0x5C) = at<std::uint32_t>(screen, 0x58);
        at<std::uint32_t>(screen, 0x8C) = at<std::uint32_t>(screen, 0x88);
        at<std::uint32_t>(screen, 0x6C) = at<std::uint32_t>(screen, 0x68);
        clone_slot = 0x58;
        icon_slot = 0x68;
        highlight_slot = 0x88;
    } else {
        at<std::uint32_t>(screen, 0x218) = at<std::uint32_t>(screen, 0x88);
        at<std::uint32_t>(screen, 0x214) = at<std::uint32_t>(screen, 0x68);
        at<std::uint32_t>(screen, 0x210) = at<std::uint32_t>(screen, 0x58);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x94), 0);
        at<std::uint32_t>(screen, 0x58) = at<std::uint32_t>(screen, 0x5C);
        at<std::uint32_t>(screen, 0x88) = at<std::uint32_t>(screen, 0x8C);
        at<std::uint32_t>(screen, 0x68) = at<std::uint32_t>(screen, 0x6C);
        at<std::uint32_t>(screen, 0x5C) = at<std::uint32_t>(screen, 0x60);
        at<std::uint32_t>(screen, 0x8C) = at<std::uint32_t>(screen, 0x90);
        at<std::uint32_t>(screen, 0x6C) = at<std::uint32_t>(screen, 0x70);
        at<std::uint32_t>(screen, 0x60) = at<std::uint32_t>(screen, 0x64);
        at<std::uint32_t>(screen, 0x90) = at<std::uint32_t>(screen, 0x94);
        at<std::uint32_t>(screen, 0x70) = at<std::uint32_t>(screen, 0x74);
        clone_slot = 0x64;
        icon_slot = 0x74;
        highlight_slot = 0x94;
    }

    void* clone = host.widget_clone_subtree_00aab4c0(at<void*>(at<void*>(in_game_gui(host), 0xBC), 0x138), nullptr);
    at<void*>(screen, clone_slot) = clone;
    host.widget_vcall_34_set_visible(clone, 1);
    with_c_string("icon_L_Icon", 0xB, host, [&](Cc13SweepBNativeString* name) {
        at<void*>(screen, icon_slot) = host.widget_find_child_00aa7e00(at<void*>(screen, clone_slot), name, 1);
    });
    with_c_string("number_Text", 0xB, host, [&](Cc13SweepBNativeString* name) {
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(screen, icon_slot), name, 1), 0);
    });
    host.widget_vcall_88(at<void*>(screen, icon_slot), 3, 0, 1.0f);

    const std::uint32_t start =
        static_cast<std::uint32_t>(static_cast<std::int32_t>(host.widget_vcall_98_short(at<void*>(screen, icon_slot))));
    auto bound = [&]() {
        return at<std::uint32_t>(at<void*>(in_game_gui(host), 0xC0), 0x1E0) + start;
    };
    for (std::uint32_t i = start; i < bound(); ++i) {
        std::int32_t key_a = static_cast<std::int32_t>(i);
        std::int32_t key_b = static_cast<std::int32_t>(i);
        void** first = host.map_subscript_006170f0(static_cast<std::uint8_t*>(screen) + 0x1D8, &key_a);
        void** second = host.map_subscript_006170f0(static_cast<std::uint8_t*>(screen) + 0x1D8, &key_b);
        host.gui_icon_add_state_00ab64c0(at<void*>(screen, icon_slot), at<void*>(*second, 0xD0),
                                         static_cast<std::uint8_t*>(*first) + 0xD4);
    }

    const std::uint32_t index = at<std::uint32_t>(screen, 0x1F4);
    void** begin = at<void**>(screen, 0x188);
    if (begin == nullptr ||
        index >= static_cast<std::uint32_t>(
                     (static_cast<std::int32_t>(at<std::uintptr_t>(screen, 0x18C) -
                                                reinterpret_cast<std::uintptr_t>(begin))) >> 2)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* record = at<void**>(screen, 0x188)[index];
    const std::uint16_t state = *static_cast<const std::uint16_t*>(
        host.string_map_ci_subscript_00617870(static_cast<std::uint8_t*>(screen) + 0x1CC, at<void*>(record, 0x4)));
    host.widget_vcall_88(at<void*>(screen, icon_slot), state, 0, 1.0f);

    with_c_string("circle_small_01_Group", 0x15, host, [&](Cc13SweepBNativeString* name) {
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(screen, clone_slot), name, 1), 0);
    });
    with_c_string("circle_small_02_Group", 0x15, host, [&](Cc13SweepBNativeString* name) {
        host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(screen, clone_slot), name, 1), 0);
    });
    with_c_string("circle_HL_Icon", 0xE, host, [&](Cc13SweepBNativeString* name) {
        at<void*>(screen, highlight_slot) = host.widget_find_child_00aa7e00(at<void*>(screen, clone_slot), name, 1);
    });
    host.widget_vcall_34_set_visible(at<void*>(screen, highlight_slot), 1);

    if (up) {
        host.widget_vcall_44_set_float(at<void*>(screen, 0x68),
                                       f32_bits(0x80000000u) -
                                           at<float>(at<void*>(in_game_gui(host), 0xBC), 0x2A4));
        host.widget_vcall_44_set_float(at<void*>(screen, 0x58), at<float>(at<void*>(in_game_gui(host), 0xBC), 0x2A4));
    } else {
        // FLD [bc+2A8h]; FMUL qword [00D7A370]; FSUB/FADD [bc+2A4h]; FSTP float
        // (00619A46..00619A61, 00619A7A..00619A95). HUD code runs on the main
        // thread at 24-bit PC (docs/CC13_FPU_PRECISION.md), so the product and
        // the sum each round to float in that order.
        const double five = f64_bits(0x4014000000000000ull);  // 00D7A370
        void* bc = at<void*>(in_game_gui(host), 0xBC);
        const float down_product = static_cast<float>(static_cast<double>(at<float>(bc, 0x2A8)) * five);
        host.widget_vcall_44_set_float(
            at<void*>(screen, 0x74),
            static_cast<float>(static_cast<double>(down_product) - static_cast<double>(at<float>(bc, 0x2A4))));
        bc = at<void*>(in_game_gui(host), 0xBC);
        const float up_product = static_cast<float>(static_cast<double>(at<float>(bc, 0x2A8)) * five);
        host.widget_vcall_44_set_float(
            at<void*>(screen, 0x64),
            static_cast<float>(static_cast<double>(up_product) + static_cast<double>(at<float>(bc, 0x2A4))));
    }
    host.hud_support_set_large_icon_alpha_0065e870(at<void*>(in_game_gui(host), 0xBC), at<void*>(screen, clone_slot),
                                                   0.0f);
    at<float>(screen, 0x1FC) = f32_bits(0x3F800000u);  // 00D7A24C
}

// ---------------------------------------------------------------------------
// 0078A430 BSP_NetworkPacketStatistics_FrameUpdate
// __fastcall(ECX = stats), RET. Image facts (asm 0078A430..0078AFEF):
// - Nothing unless byte +762900h. n = +76296Ch; +76296Ch = n + 1; when
//   n % 5 == 0 (DIV by 5):
//   * 1001 rows R(r) = +74F02Ch + 50h*r, each with source cells
//     +18h + 1DE8h*r + 10h*(t-1) for t = 1..E9h: words R+0Ch/R+1Ch = 9999 and
//     the rest of R+0h..R+1Eh cleared, then per cell: count [c] != 0 adds
//     into R+4h (and [c-4] into R+0h); (unsigned)(short)R+0Eh < count stores
//     the count's low word in R+0Eh and t in R+8h, and when +7648BCh <
//     (unsigned)(short)count the global maximum +7648E4h = t, +7648BCh =
//     (short)R+0Eh; the same for the second direction ([c+EF4h] into R+14h,
//     [c+EF0h] into R+10h, max word R+1Eh, type R+18h, no global update).
//   * 100 rates from rows 10, 20, .., 1000 (EDX = +74F34Ch + 320h*k) into
//     +76299Ch + 50h*k: the image adds the SAME row's unsigned fields ten
//     times (float stores after every add; the source loop never advances);
//     +0h/+28h = sum(R+4h) * 0.125 when sum(R+0h) > 0 (FCOMIP, JBE -> 0),
//     +4h/+2Ch = sum(R+14h) * 0.125 when sum(R+10h) > 0 (COMISS, JBE -> 0).
//   Unsigned dwords become floats as FILD + 2^32 (00CE3978) when negative.
// - Byte +7649A8h: 007884D0(ECX = stats).
// - Unless byte +764934h: ten scale lines i = 0..9 at y = 0.4 - i*1024.0 / S
//   (S = float +762970h) from x -0.5f to 0.5f in (0.6f, 0.6f, 0.6f, 1.0f),
//   each labelled with ((+76295Ch - +762958h) / 5) * i + +762958h through
//   00711370 and 004C7070([00E188A8], 10 + 256*i, 729.6f, text) (text or
//   00F871B8 when null), the temporary released after.
// - The graph, i = 1..1000 over R(i): hA = maxA / S, vA = countA / S,
//   hB = maxB / S, vB = countB / S (S read once); S grows by 1024.0 while
//   vA > S * 0.3, then while vB > S * 0.3 (float store each step); the two
//   bar pairs are drawn larger first (vA > vB: A then B, else B then A);
//   every fifth i joins the previous rate to the current one (+762974h + 8i
//   and +762978h + 8i, each only when > 0) and keeps it as the previous.
//   Colours are locals with alpha +764938h re-read for each.
// Operand directions were read from the bytes: DE FA = FDIVP ST2 (ST2/ST0),
// D8 F1 = FDIV ST1 (ST0/ST1), DE E3 / DE E1 = FSUBRP (ST0 - STi),
// DE F2 = FDIVRP ST2 (ST0/ST2), DC E9 = FSUB ST1,ST0 (ST1 - ST0).
// ---------------------------------------------------------------------------
namespace {

float unsigned_to_float(std::uint32_t value) {
    return static_cast<float>(static_cast<double>(value));
}

// The sum the image unrolls: 0 + v, then v added nine more times.
float ten_times(float value) {
    float sum = value + 0.0f;
    for (int k = 0; k < 9; ++k) sum = sum + value;
    return sum;
}

}  // namespace

void cc13_sweepB7_packet_stats_frame_update_0078a430(void* stats, Cc13SweepB7Host& host) {
    std::uint8_t* const base = static_cast<std::uint8_t*>(stats);
    if (at<std::uint8_t>(stats, 0x762900) == 0) return;

    const std::uint32_t calls = at<std::uint32_t>(stats, 0x76296C);
    at<std::uint32_t>(stats, 0x76296C) = calls + 1;
    if (calls % 5 == 0) {
        for (std::uint32_t r = 0; r < 0x3E9; ++r) {
            std::uint8_t* row = base + 0x74F02C + 0x50 * r;
            at<std::uint16_t>(row, 0xC) = 9999;
            at<std::uint16_t>(row, 0xE) = 0;
            at<std::uint32_t>(row, 0x8) = 0;
            at<std::uint32_t>(row, 0x4) = 0;
            at<std::uint32_t>(row, 0x0) = 0;
            at<std::uint16_t>(row, 0x1C) = 9999;
            at<std::uint16_t>(row, 0x1E) = 0;
            at<std::uint32_t>(row, 0x18) = 0;
            at<std::uint32_t>(row, 0x14) = 0;
            at<std::uint32_t>(row, 0x10) = 0;
            std::uint8_t* cell = base + 0x18 + 0x1DE8 * r;
            for (std::uint32_t t = 1; t < 0xEA; ++t, cell += 0x10) {
                if (at<std::uint32_t>(cell, 0) != 0) {
                    at<std::uint32_t>(row, 0x4) += at<std::uint32_t>(cell, 0);
                    at<std::uint32_t>(row, 0x0) += *reinterpret_cast<std::uint32_t*>(cell - 4);
                    if (static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::int16_t>(row, 0xE))) <
                        at<std::uint32_t>(cell, 0)) {
                        const std::uint16_t low = static_cast<std::uint16_t>(at<std::uint32_t>(cell, 0));
                        at<std::uint16_t>(row, 0xE) = low;
                        at<std::uint32_t>(row, 0x8) = t;
                        if (at<std::uint32_t>(stats, 0x7648BC) <
                            static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int16_t>(low)))) {
                            at<std::uint32_t>(stats, 0x7648E4) = t;
                            at<std::int32_t>(stats, 0x7648BC) = at<std::int16_t>(row, 0xE);
                        }
                    }
                }
                if (at<std::uint32_t>(cell, 0xEF4) != 0) {
                    at<std::uint32_t>(row, 0x14) += at<std::uint32_t>(cell, 0xEF4);
                    at<std::uint32_t>(row, 0x10) += at<std::uint32_t>(cell, 0xEF0);
                    if (static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::int16_t>(row, 0x1E))) <
                        at<std::uint32_t>(cell, 0xEF4)) {
                        at<std::uint16_t>(row, 0x1E) = static_cast<std::uint16_t>(at<std::uint32_t>(cell, 0xEF4));
                        at<std::uint32_t>(row, 0x18) = t;
                    }
                }
            }
        }
        for (std::uint32_t k = 0; k < 100; ++k) {
            std::uint8_t* row = base + 0x74F34C + 0x320 * k;
            std::uint8_t* out = base + 0x76299C + 0x50 * k;
            const float sum_count_a = ten_times(unsigned_to_float(at<std::uint32_t>(row, 0x4)));
            const float sum_bytes_a = ten_times(unsigned_to_float(at<std::uint32_t>(row, 0x0)));
            const float sum_count_b = ten_times(unsigned_to_float(at<std::uint32_t>(row, 0x14)));
            const float sum_bytes_b = ten_times(unsigned_to_float(at<std::uint32_t>(row, 0x10)));
            const float rate_a = sum_bytes_a > 0.0f ? static_cast<float>(sum_count_a * 0.125) : 0.0f;
            at<float>(out, 0x28) = rate_a;
            at<float>(out, 0x0) = rate_a;
            const float rate_b = sum_bytes_b > 0.0f ? static_cast<float>(sum_count_b * 0.125) : 0.0f;
            at<float>(out, 0x2C) = rate_b;
            at<float>(out, 0x4) = rate_b;
        }
    }

    if (at<std::uint8_t>(stats, 0x7649A8) != 0) host.packet_stats_write_csv_row_007884d0(stats);
    if (at<std::uint8_t>(stats, 0x764934) != 0) return;

    for (std::int32_t i = 0, x = 10; x < 0xA0A; ++i, x += 0x100) {
        const float grey[4] = {f32_bits(0x3F19999Au), f32_bits(0x3F19999Au), f32_bits(0x3F19999Au),
                               f32_bits(0x3F800000u)};
        const float y = static_cast<float>(0.4 - static_cast<double>(i) * 1024.0 / at<float>(stats, 0x762970));
        host.debug_draw_screen_line_00860030(grey, f32_bits(0xBF000000u), y, f32_bits(0x3F000000u), y);
        const std::uint32_t low = at<std::uint32_t>(stats, 0x762958);
        Cc13SweepBNativeString label;  // constructed by 00711370
        void* text = host.native_string_from_unsigned_00711370(
            &label, ((at<std::uint32_t>(stats, 0x76295C) - low) / 5) * static_cast<std::uint32_t>(i) + low);
        const char* chars = static_cast<Cc13SweepBNativeString*>(text)->data;
        if (chars == nullptr) chars = static_cast<const char*>(host.image_global(0x00F871B8));
        host.diagnostic_printf_004c7070(*static_cast<void**>(host.image_global(0x00E188A8)), static_cast<float>(x),
                                        f32_bits(0x44366666u), chars);
        release_string(label, host);
    }

    const double k_grow = 0.3;                   // 00D04368
    const double k_step = 0.0010000000474974513;  // 00D7A318
    const double k_x_a = 0.5;                    // 00D7A280
    const double k_x_b = 0.49799999594688416;    // 00D04360
    const double k_base_a = 0.20000000298023224;  // 00CE3D10
    const double k_base_b = 0.4000000059604645;   // 00CE65D0
    float previous_a = 0.0f;
    float previous_b = 0.0f;
    for (std::int32_t i = 1; i < 0x3E9; ++i) {
        std::uint8_t* row = base + 0x74F02C + 0x50 * i;
        const float scale = at<float>(stats, 0x762970);
        const float h_a = static_cast<float>(static_cast<double>(at<std::int16_t>(row, 0xE)) / scale);
        const float v_a = static_cast<float>(static_cast<double>(at<std::uint32_t>(row, 0x4)) / scale);
        const float h_b = static_cast<float>(static_cast<double>(at<std::int16_t>(row, 0x1E)) / scale);
        const float v_b = static_cast<float>(static_cast<double>(at<std::uint32_t>(row, 0x14)) / scale);
        if (v_a > scale * k_grow) {
            do {
                at<float>(stats, 0x762970) = static_cast<float>(at<float>(stats, 0x762970) + 1024.0);
            } while (v_a > at<float>(stats, 0x762970) * k_grow);
        }
        if (v_b > at<float>(stats, 0x762970) * k_grow) {
            do {
                at<float>(stats, 0x762970) = static_cast<float>(at<float>(stats, 0x762970) + 1024.0);
            } while (v_b > at<float>(stats, 0x762970) * k_grow);
        }

        const double x = k_step * i;
        auto draw_a = [&]() {
            const float x_a = static_cast<float>(x - k_x_a);
            const float y_a = static_cast<float>(k_base_a - h_a);
            const float lower[4] = {0.0f, f32_bits(0x3F000000u), 0.0f, at<float>(stats, 0x764938)};
            host.debug_draw_screen_line_00860030(lower, x_a, f32_bits(0x3E4CCCCDu), x_a, y_a);
            const float y_top = static_cast<float>((k_base_a - v_a) - h_a);
            const float upper[4] = {0.0f, f32_bits(0x3ECCCCCDu), 0.0f, at<float>(stats, 0x764938)};
            host.debug_draw_screen_line_00860030(upper, x_a, y_a, x_a, y_top);
        };
        auto draw_b = [&]() {
            const float x_b = static_cast<float>(x - k_x_b);
            const float y_b = static_cast<float>(k_base_b - h_b);
            const float lower[4] = {f32_bits(0x3ECCCCCDu), 0.0f, 0.0f, at<float>(stats, 0x764938)};
            host.debug_draw_screen_line_00860030(lower, x_b, f32_bits(0x3ECCCCCDu), x_b, y_b);
            const float y_top = static_cast<float>((k_base_b - v_b) - h_b);
            const float upper[4] = {f32_bits(0x3F000000u), 0.0f, 0.0f, at<float>(stats, 0x764938)};
            host.debug_draw_screen_line_00860030(upper, x_b, y_b, x_b, y_top);
        };
        if (v_a > v_b) {
            draw_a();
            draw_b();
        } else {
            draw_b();
            draw_a();
        }

        if (i % 5 == 0) {
            const float x0 = static_cast<float>(static_cast<double>(i - 5) * k_step - k_x_a);
            const float x1 = static_cast<float>(x - k_x_a);
            const float rate_a = at<float>(stats, 0x762974 + 8 * i);
            if (rate_a > 0.0f) {
                const float s = at<float>(stats, 0x762970);
                const float colour[4] = {0.0f, f32_bits(0x3F666666u), 0.0f, at<float>(stats, 0x764938)};
                host.debug_draw_screen_line_00860030(colour, x0, static_cast<float>(k_base_a - static_cast<double>(previous_a) / s), x1,
                                                     static_cast<float>(k_base_a - static_cast<double>(rate_a) / s));
                previous_a = at<float>(stats, 0x762974 + 8 * i);
            }
            const float rate_b = at<float>(stats, 0x762978 + 8 * i);
            if (rate_b > 0.0f) {
                const float s = at<float>(stats, 0x762970);
                const float colour[4] = {f32_bits(0x3F666666u), 0.0f, 0.0f, at<float>(stats, 0x764938)};
                host.debug_draw_screen_line_00860030(colour, x0, static_cast<float>(k_base_b - static_cast<double>(previous_b) / s), x1,
                                                     static_cast<float>(k_base_b - static_cast<double>(rate_b) / s));
                previous_b = at<float>(stats, 0x762978 + 8 * i);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// 005A6AD0 BSP_HudMap_CreateUnitMarkerModel (constructor)
// __thiscall(ECX = marker, unit, scene, shadow_fade_source), RET 0Ch; EAX =
// marker. EH handler 00C7198C (states 0, 0Ch..14h; the unwind of the member
// vectors is noted, not modelled). Image facts (asm 005A6AD0..005A7541):
// - Stores in image order: vtable 00CF00ACh; the 4-dword members from +78h
//   (+7Ch..+84h, .., +ECh..+F4h) and +150h (+154h..+19Ch) get their three
//   pointers zeroed; byte +2Eh = 0; +4h = unit; +10h, +0Ch, +128h, +124h,
//   +130h, +12Ch, +138h, +134h, +18h, +1Ch, +48h..+6Ch = 0; byte +9h = 0;
//   +24h = 0.
// - kind = unit+C4h. Texture table +14h (G = [00E198C4]+54h, re-read in each
//   arm): unit +5Ch(6) -> G+468h; kind 18h -> kind = [unit+3D0h]+C4h and
//   G+474h; +5Ch(0Fh) -> G+474h; kind 1Ch/45h/46h/1Bh -> G+480h; kind
//   2Bh/34h -> G+48Ch; otherwise unchanged (0).
// - Model template: G[130h + 4*kind]; kind 1Bh -> G[438h + 4*[[unit+538h]+17Ch]];
//   kind 10h -> the first child (list [u+48h], next +44h; u = [unit+3D0h]
//   when +5Ch(18h)) that is +5Ch(25h) and whose 006E40F0 device answers
//   +8h(31h) selects G'[464h]; kind 1Ch -> G[450h + 4*[unit+770h]]; else when
//   +5Ch(8): byte +2Eh = no bomb-platform children (00852350), no torpedo
//   barrels (00818300) and not [[unit+538h]+C8h] > 0 (signed, read first);
//   when set, G'[1DCh] and +14h = G'+48Ch.
// - Template present: +0Ch = template +8h(6, 1.0f); +124h = +0Ch's +0Ch node;
//   00B6D890(node, scene); +12Ch = node, or node+34h when node +0Ch(00B74310())
//   fails; +134h = 005A07B0(+12Ch, "mapmodel"); 00B18B20(+134h, "cShadowFade",
//   shadow_fade_source). The same for G[2B4h + 4*kind] into +10h/+128h/+130h/
//   +138h when present. +13Ch = 0; when 00B72B40(00B74640(+12Ch, 0)) > 1:
//   +13Ch = 005A07B0(+12Ch, "alphablend"), +140h = 0.0f, 00B18B20(+13Ch,
//   "cAlpha", &+140h). When +0Ch: index = [+4h]+54h, or 2 when 00803CE0(party
//   [[00E188A8]+18CCh+4*[[00E188A8]+18ECh]]+28h, +4h) == 3; 00B189F0(+134h,
//   0, [+14h][index]) and the same for +138h when non-null. When byte
//   unit+C8h is clear the unit world matrix unit+CCh is rebuilt (00414DB0 on
//   [unit+3Ch] when set, unit+74h times [unit+3Ch]+CCh through 00413920 or
//   unit+74h as is, copied, byte +C8h = 1, byte +10Ch = 0). Then
//   diag(0.001f, 0.001f, 0.001f, 1) (00D7A23C) times unit+CCh, with row 3
//   set to the unit +C4h position plus 1500.0 (double 00CF00F8) on y, goes to
//   +124h and +128h vcall +34h when they are set.
// - Always: the +E8h vector is erased whole (the inlined checked erase:
//   00BF6713 when first > last, memmove_s 00BF67A7 of the empty tail),
//   resized to 11 (005A5A80(11, 0)) and each element nulled by checked
//   index; +118h, +20h = 0; +3Ch = 0.0f; bytes +28h..+2Dh = 0; +38h = 0;
//   +40h, +44h = 0.0f; +34h = +30h = [[unit+284h]+14h] when 007788B0(unit),
//   else 0.
// Strings: the temporaries release the data pointer read before the call
// (the helper re-reads it after; the callees take the names as const).
// ---------------------------------------------------------------------------
void* cc13_sweepB7_hud_map_unit_marker_construct_005a6ad0(void* marker, void* unit, void* scene,
                                                          void* shadow_fade_source, Cc13SweepB7Host& host) {
    auto gui_block = [&]() { return at<std::uint8_t*>(in_game_gui(host), 0x54); };

    at<std::uint32_t>(marker, 0x0) = 0x00CF00ACu;  // the image's vtable address, kept as a value
    for (std::size_t member = 0x78; member <= 0xE8; member += 0x10) {
        at<std::uint32_t>(marker, member + 0x4) = 0;
        at<std::uint32_t>(marker, member + 0x8) = 0;
        at<std::uint32_t>(marker, member + 0xC) = 0;
    }
    for (std::size_t member = 0x150; member <= 0x190; member += 0x10) {
        at<std::uint32_t>(marker, member + 0x4) = 0;
        at<std::uint32_t>(marker, member + 0x8) = 0;
        at<std::uint32_t>(marker, member + 0xC) = 0;
    }
    at<std::uint8_t>(marker, 0x2E) = 0;
    at<void*>(marker, 0x4) = unit;
    at<std::uint32_t>(marker, 0x10) = 0;
    at<std::uint32_t>(marker, 0xC) = 0;
    at<std::uint32_t>(marker, 0x128) = 0;
    at<std::uint32_t>(marker, 0x124) = 0;
    at<std::uint32_t>(marker, 0x130) = 0;
    at<std::uint32_t>(marker, 0x12C) = 0;
    at<std::uint32_t>(marker, 0x138) = 0;
    at<std::uint32_t>(marker, 0x134) = 0;
    at<std::uint32_t>(marker, 0x18) = 0;
    at<std::uint32_t>(marker, 0x1C) = 0;
    for (std::size_t offset = 0x48; offset <= 0x6C; offset += 4) at<std::uint32_t>(marker, offset) = 0;
    at<std::uint8_t>(marker, 0x9) = 0;
    at<std::uint32_t>(marker, 0x24) = 0;

    std::int32_t kind = at<std::int32_t>(unit, 0xC4);
    if (host.entity_vcall_5c_is_kind(unit, 6)) {
        at<std::uint8_t*>(marker, 0x14) = gui_block() + 0x468;
    } else if (kind == 0x18) {
        kind = at<std::int32_t>(at<void*>(unit, 0x3D0), 0xC4);
        at<std::uint8_t*>(marker, 0x14) = gui_block() + 0x474;
    } else if (host.entity_vcall_5c_is_kind(unit, 0xF)) {
        at<std::uint8_t*>(marker, 0x14) = gui_block() + 0x474;
    } else if (kind == 0x1C || kind == 0x45 || kind == 0x46 || kind == 0x1B) {
        at<std::uint8_t*>(marker, 0x14) = gui_block() + 0x480;
    } else if (kind == 0x2B || kind == 0x34) {
        at<std::uint8_t*>(marker, 0x14) = gui_block() + 0x48C;
    }

    std::uint8_t* block = gui_block();
    void* model_template = at<void*>(block, 0x130 + 4 * kind);
    if (kind == 0x1B) {
        model_template = at<void*>(block, 0x438 + 4 * at<std::int32_t>(at<void*>(unit, 0x538), 0x17C));
    } else if (kind == 0x10) {
        void* owner = unit;
        if (host.entity_vcall_5c_is_kind(unit, 0x18)) owner = at<void*>(unit, 0x3D0);
        for (void* child = at<void*>(owner, 0x48); child != nullptr; child = at<void*>(child, 0x44)) {
            if (host.entity_vcall_5c_is_kind(child, 0x25) &&
                host.device_vcall_08_is_kind(host.unit_bomb_rack_loaded_device_006e40f0(child), 0x31)) {
                model_template = at<void*>(gui_block(), 0x464);
                break;
            }
        }
    } else if (kind == 0x1C) {
        model_template = at<void*>(block, 0x450 + 4 * at<std::int32_t>(unit, 0x770));
    } else if (host.entity_vcall_5c_is_kind(unit, 8)) {
        const bool has_loadout = at<std::int32_t>(at<void*>(unit, 0x538), 0xC8) > 0;
        const bool has_torpedoes = host.unit_torpedo_barrel_count_00818300(unit) != 0;
        const bool has_bombs = host.unit_count_bomb_platform_children_00852350(unit) != 0;
        const bool unarmed = !has_bombs && !has_torpedoes && !has_loadout;
        at<std::uint8_t>(marker, 0x2E) = unarmed ? 1 : 0;
        if (unarmed) {
            std::uint8_t* current = gui_block();
            model_template = at<void*>(current, 0x1DC);
            at<std::uint8_t*>(marker, 0x14) = current + 0x48C;
        }
    }

    if (model_template != nullptr) {
        // One model: create, register under the scene, resolve the generated
        // model and bind its "mapmodel" material's cShadowFade.
        auto create = [&](void* source, std::size_t object_slot, std::size_t node_slot, std::size_t model_slot,
                          std::size_t material_slot) {
            void* object = host.factory_vcall_08_create(source, 6, 1.0f);
            at<void*>(marker, object_slot) = object;
            at<void*>(marker, node_slot) = at<void*>(object, 0xC);
            host.node_propagate_root_00b6d890(at<void*>(marker, node_slot), scene);
            void* node = at<void*>(marker, node_slot);
            const bool generated = host.node_vcall_0c_is_kind_of(node, host.generated_model_current_type_00b74310());
            void* model = at<void*>(marker, node_slot);
            if (!generated) model = at<void*>(model, 0x34);
            at<void*>(marker, model_slot) = model;
            with_c_string("mapmodel", 8, host, [&](Cc13SweepBNativeString* name) {
                at<void*>(marker, material_slot) =
                    host.generated_model_find_material_005a07b0(at<void*>(marker, model_slot), name);
            });
            with_c_string("cShadowFade", 0xB, host, [&](Cc13SweepBNativeString* name) {
                host.material_register_float_parameter_00b18b20(at<void*>(marker, material_slot), name,
                                                               shadow_fade_source);
            });
        };
        create(model_template, 0xC, 0x124, 0x12C, 0x134);
        void* second_template = at<void*>(gui_block(), 0x2B4 + 4 * kind);
        if (second_template != nullptr) create(second_template, 0x10, 0x128, 0x130, 0x138);

        at<std::uint32_t>(marker, 0x13C) = 0;
        if (host.mesh_object_count58_00b72b40(host.generated_model_geometry_00b74640(at<void*>(marker, 0x12C), 0)) >
            1) {
            with_c_string("alphablend", 0xA, host, [&](Cc13SweepBNativeString* name) {
                at<void*>(marker, 0x13C) = host.generated_model_find_material_005a07b0(at<void*>(marker, 0x12C), name);
            });
            at<float>(marker, 0x140) = 0.0f;
            with_c_string("cAlpha", 6, host, [&](Cc13SweepBNativeString* name) {
                host.material_register_float_parameter_00b18b20(at<void*>(marker, 0x13C), name,
                                                               static_cast<std::uint8_t*>(marker) + 0x140);
            });
        }

        if (at<void*>(marker, 0xC) != nullptr) {
            std::uint8_t* game = *static_cast<std::uint8_t**>(host.image_global(0x00E188A8));
            void* party = at<void*>(at<void*>(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC)), 0x28);
            std::int32_t index = at<std::int32_t>(at<void*>(marker, 0x4), 0x54);
            if (host.recon_relation_if_identified_00803ce0(party, at<void*>(marker, 0x4)) == 3) index = 2;
            host.material_set_texture_slot_00b189f0(at<void*>(marker, 0x134), 0,
                                                    at<void**>(marker, 0x14)[index]);
            if (at<void*>(marker, 0x138) != nullptr) {
                host.material_set_texture_slot_00b189f0(at<void*>(marker, 0x138), 0,
                                                        at<void**>(marker, 0x14)[index]);
            }
        }

        float* world = &at<float>(unit, 0xCC);
        if (at<std::uint8_t>(unit, 0xC8) == 0) {
            if (at<void*>(unit, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(at<void*>(unit, 0x3C));
            float product[16];
            const float* source = &at<float>(unit, 0x74);
            if (at<void*>(unit, 0x3C) != nullptr) {
                source = host.matrix_multiply_00413920(&at<float>(unit, 0x74), product,
                                                       &at<float>(at<void*>(unit, 0x3C), 0xCC));
            }
            for (int k = 0; k < 16; ++k) world[k] = source[k];
            at<std::uint8_t>(unit, 0xC8) = 1;
            at<std::uint8_t>(unit, 0x10C) = 0;
        }
        const float s = f32_bits(0x3A83126Fu);  // 00D7A23C
        const float scale[16] = {s, 0.0f, 0.0f, 0.0f, 0.0f, s, 0.0f, 0.0f,
                                 0.0f, 0.0f, s, 0.0f, 0.0f, 0.0f, 0.0f, f32_bits(0x3F800000u)};
        float placed[16];
        host.matrix_multiply_00413920(scale, placed, world);
        float position_out[3];
        const float* position = host.unit_vcall_c4_position(unit, position_out);
        placed[12] = position[0];
        placed[13] = position[1];
        placed[14] = position[2];
        placed[13] = static_cast<float>(static_cast<double>(placed[13]) + 1500.0);  // 00CF00F8
        if (at<void*>(marker, 0x124) != nullptr) host.node_vcall_34_set_matrix(at<void*>(marker, 0x124), placed);
        if (at<void*>(marker, 0x128) != nullptr) host.node_vcall_34_set_matrix(at<void*>(marker, 0x128), placed);
    }

    // The inlined erase(begin(), end()) of the vector at +E8h.
    std::uint8_t* const vector = static_cast<std::uint8_t*>(marker) + 0xE8;
    std::uint8_t* const end_iter = at<std::uint8_t*>(vector, 0x8);
    if (at<std::uint8_t*>(vector, 0x4) > end_iter) host.crt_invalid_parameter_noinfo_00bf6713();
    std::uint8_t* const first = at<std::uint8_t*>(vector, 0x4);
    if (first > at<std::uint8_t*>(vector, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
    if (first != end_iter) {
        const std::int32_t count = static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x8) - end_iter) >> 2;
        const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4;
        if (count > 0) host.crt_memmove_s_00bf67a7(first, bytes, end_iter, bytes);
        at<std::uint8_t*>(vector, 0x8) = first + bytes;
    }
    host.pointer_vector_resize_005a5a80(vector, 0xB, 0);
    for (std::uint32_t k = 0;; ++k) {
        void** data = at<void**>(vector, 0x4);
        if (data == nullptr ||
            k >= static_cast<std::uint32_t>(
                     static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x8) - reinterpret_cast<std::uint8_t*>(data)) >>
                     2)) {
            break;
        }
        // The inner checked index repeats the same test (00BF6713 on failure).
        at<void**>(vector, 0x4)[k] = nullptr;
    }

    at<std::uint32_t>(marker, 0x118) = 0;
    at<std::uint32_t>(marker, 0x20) = 0;
    at<float>(marker, 0x3C) = 0.0f;
    for (std::size_t offset = 0x28; offset <= 0x2D; ++offset) at<std::uint8_t>(marker, offset) = 0;
    at<std::uint32_t>(marker, 0x38) = 0;
    at<float>(marker, 0x40) = 0.0f;
    at<float>(marker, 0x44) = 0.0f;
    std::uint32_t leader = 0;
    if (host.unit_is_formation_follower_007788b0(unit)) leader = at<std::uint32_t>(at<void*>(unit, 0x284), 0x14);
    at<std::uint32_t>(marker, 0x34) = leader;
    at<std::uint32_t>(marker, 0x30) = leader;
    return marker;
}

// ---------------------------------------------------------------------------
// 005BE240 BSP_HudMinimapScreen_LoadLayout
// __fastcall(ECX = screen), RET; EBP frame aligned to 8, EH handler 00C726D8
// (states 0..n track the name temporaries). Image facts (asm
// 005BE240..005BEC18), every lookup 00AA7E00(parent, name, 1):
// - +4Ch = "minimap_compass_Icon" under the root +1Ch.
// - "minimap_units_white_Group" under the root, hidden (+34h(0)); under it
//   +6Ch/+70h/+74h/+78h/+7Ch = item_ship_Icon / item_plane_Icon /
//   item_sub_Icon / item_building_Icon / item_marker_Icon; +D0h..+DCh = the
//   four dwords of +6Ch vcall +54h(out) (the colour).
// - For red (+80h), blue (+90h), grey (+C0h), yellow (+A0h), silver (+B0h),
//   in that order: "minimap_units_<c>_Group" under the root, hidden, and the
//   four dwords of its item_ship_Icon's +54h(out).
// - +48h = "minimap_dir_Icon", +F0h = "unit_marker_Group", under the root.
// Each name temporary is released before the result is used.
// ---------------------------------------------------------------------------
void cc13_sweepB7_hud_minimap_load_layout_005be240(void* screen, Cc13SweepB7Host& host) {
    auto find = [&](void* parent, const char* name) {
        void* child = nullptr;
        with_c_string(name, static_cast<std::uint32_t>(std::strlen(name)), host,
                      [&](Cc13SweepBNativeString* text) { child = host.widget_find_child_00aa7e00(parent, text, 1); });
        return child;
    };
    auto copy_colour = [&](void* icon, std::size_t offset) {
        float out[4];
        const std::uint32_t* colour =
            reinterpret_cast<const std::uint32_t*>(host.widget_vcall_54_colour(icon, out));
        for (std::size_t k = 0; k < 4; ++k) at<std::uint32_t>(screen, offset + 4 * k) = colour[k];
    };

    at<void*>(screen, 0x4C) = find(at<void*>(screen, 0x1C), "minimap_compass_Icon");
    void* white = find(at<void*>(screen, 0x1C), "minimap_units_white_Group");
    host.widget_vcall_34_set_visible(white, 0);
    at<void*>(screen, 0x6C) = find(white, "item_ship_Icon");
    at<void*>(screen, 0x70) = find(white, "item_plane_Icon");
    at<void*>(screen, 0x74) = find(white, "item_sub_Icon");
    at<void*>(screen, 0x78) = find(white, "item_building_Icon");
    at<void*>(screen, 0x7C) = find(white, "item_marker_Icon");
    copy_colour(at<void*>(screen, 0x6C), 0xD0);

    struct Group {
        const char* name;
        std::size_t colour;
    };
    const Group groups[] = {{"minimap_units_red_Group", 0x80},    {"minimap_units_blue_Group", 0x90},
                            {"minimap_units_grey_Group", 0xC0},   {"minimap_units_yellow_Group", 0xA0},
                            {"minimap_units_silver_Group", 0xB0}};
    for (const Group& group : groups) {
        void* widget = find(at<void*>(screen, 0x1C), group.name);
        host.widget_vcall_34_set_visible(widget, 0);
        copy_colour(find(widget, "item_ship_Icon"), group.colour);
    }

    at<void*>(screen, 0x48) = find(at<void*>(screen, 0x1C), "minimap_dir_Icon");
    at<void*>(screen, 0xF0) = find(at<void*>(screen, 0x1C), "unit_marker_Group");
}

// ---------------------------------------------------------------------------
// 0065BF00 (support-manager map icon pass)
// __fastcall(ECX = screen), RET; EBP frame aligned to 8, EH handler 00C7B028
// (state 0 owns the 4Ch slot-entry copy of the unit path). Image facts (asm
// 0065BF00..0065C99B):
// - +128h vcall +34h(+5Ch == 0). Returns when byte +2DCh is set and +2DDh
//   is clear; clears +2DDh; mode = 004BCA50([00E188A8]).
// - +34Ch set: +448h..+454h <- +438h..+444h, which are cleared; dword +6Ch =
//   0; n = +3E8h into +298h. n == 0: modes 4..7 hide slot 0, others hide
//   all four. Modes 4..7 with n: slot 0 shown, alpha 0.6f (00CE3D30), frame
//   00652C80 of the list's first entry (00BF6713 when the list is empty).
//   Otherwise: +2C4h is clamped to n - 1 (unsigned), the walk takes +2C4h
//   steps from the first node, +68h = +324h, frames[+324h] = 00652C80 of
//   the node (node+8h..+18h by value); a node with byte +14h set stores the
//   unit's word +174h in +438h[+324h] and places +128h at 00658580 of the
//   unit position (00414DB0 first when byte +C8h is clear) and shows it,
//   else -1 and +128h hidden. Then backwards from +324h - 1 to 0 (cyclic,
//   skipping the head) and forwards from +324h + 1 while < 4 and < n
//   (cyclic), each with +68h = index, its frame and its +438h id. Every
//   node dereference has the checked-iterator test (00BF6713 when the
//   node is the head or the container is null / differs).
//   Finally for each filled slot k: +88h(word frames[k], 0, 1.0f), alpha
//   1.0f when +438h[k] > 0 else 0.6f, and +1E0h when byte +6Ch[k] is set.
// - +34Ch clear: nothing unless +1F4h. The selected iterator +300h (00BF6713
//   checks); its unit +5Ch(45h) or 00658A60's value +5Ch(9) selects the
//   group path; otherwise +298h = 0, the four slots shown, and for each
//   4Ch entry k of unit+794h..+798h (count re-read every pass): the entry is
//   copied (00846FD0), +128h placed at the entry's unit +30h when +5Ch is 0
//   and k == +324h (hidden when that unit is null), slot k gets frame
//   00652BA0(entry) with alpha 0.5f for state 1/2 else 1.0f and +298h
//   counts it, or is hidden when the state +4h is 0; the copy is destroyed
//   (0065BB00).
// - Group path: +298h = 0; the group is value+72Ch (45h) or value+1188h, or
//   null; per record i (58h bytes from [g+4Ch]) the count [g+50h] is read
//   under the group lock [g+0Ch] (Enter/Leave CriticalSection with the depth
//   +18h raised around it, when the lock is set); +128h is placed for the
//   selected record when its observed unit +28h is set with state 3/4, or
//   the state is 2/5 (hidden when +28h is null); state 1 hides slot i, else
//   it is shown with frame 00658620(record) and alpha 0.35f (00CF6560) for
//   state 2, +1E0h for state 1/5 or an out record (00658530) whose +28h
//   +174h() is 00E08FA0, 1.0f otherwise.
// frames is the image's four-int local; +324h above 3 would overrun it in
// the image as well (not guarded here either).
// ---------------------------------------------------------------------------
namespace {

// The checked iterator test the image inlines before each node use.
void list_check_node(void* list, void* node, Cc13SweepB7Host& host) {
    if (node == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
}

// 00658580 at the unit's world position (refreshed first when byte +C8h is
// clear), then 00AA7D00 on the selection marker +128h.
void place_selection_marker(void* screen, void* unit, Cc13SweepB7Host& host) {
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    float out[2];
    host.hud_support_world_to_map_00658580(screen, out, at<float>(unit, 0xFC), at<float>(unit, 0x100),
                                           at<float>(unit, 0x104));
    host.widget_set_local_xy_00aa7d00(at<void*>(screen, 0x128), out[0], out[1]);
}

// 00652C80 of a list node: the five dwords node+8h..+18h passed by value.
std::int32_t node_frame(void* screen, void* node, Cc13SweepB7Host& host) {
    return cc13_sweepB4_hud_unit_icon_frame_00652c80(screen, at<void*>(node, 0x8), at<std::int32_t>(node, 0xC),
                                                     at<std::int32_t>(node, 0x10), at<std::int32_t>(node, 0x14),
                                                     at<void*>(node, 0x18), host);
}

}  // namespace

void cc13_sweepB7_support_map_icon_pass_0065bf00(void* screen, Cc13SweepB7Host& host) {
    std::uint8_t* const base = static_cast<std::uint8_t*>(screen);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), at<std::uint32_t>(screen, 0x5C) == 0 ? 1 : 0);
    if (at<std::uint8_t>(screen, 0x2DC) != 0 && at<std::uint8_t>(screen, 0x2DD) == 0) return;
    if (at<std::uint8_t>(screen, 0x2DD) != 0) at<std::uint8_t>(screen, 0x2DD) = 0;
    const std::int32_t mode =
        host.game_effective_mode_004bca50(*static_cast<void**>(host.image_global(0x00E188A8)));
    const bool mode_4_to_7 = mode == 4 || mode == 5 || mode == 6 || mode == 7;
    const float dim = f32_bits(0x3F19999Au);  // 00CE3D30, 0.6f

    if (at<std::uint32_t>(screen, 0x34C) != 0) {
        at<std::uint32_t>(screen, 0x448) = at<std::uint32_t>(screen, 0x438);
        at<std::uint32_t>(screen, 0x44C) = at<std::uint32_t>(screen, 0x43C);
        at<std::uint32_t>(screen, 0x438) = 0;
        at<std::uint32_t>(screen, 0x43C) = 0;
        at<std::uint32_t>(screen, 0x450) = at<std::uint32_t>(screen, 0x440);
        at<std::uint32_t>(screen, 0x440) = 0;
        at<std::uint32_t>(screen, 0x454) = at<std::uint32_t>(screen, 0x444);
        at<std::uint32_t>(screen, 0x444) = 0;
        at<std::uint32_t>(screen, 0x6C) = 0;
        const std::uint32_t n = at<std::uint32_t>(screen, 0x3E8);
        at<std::uint32_t>(screen, 0x298) = n;
        if (n == 0) {
            if (mode_4_to_7) {
                host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC), 0);
                return;
            }
            for (std::size_t k = 0; k < 4; ++k) host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC + 4 * k), 0);
            return;
        }
        if (mode_4_to_7) {
            host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC), 1);
            host.widget_vcall_4c_set_alpha(at<void*>(screen, 0xBC), dim);
            void* head = at<void*>(screen, 0x3E4);
            void* first = at<void*>(head, 0x0);
            if (first == head) host.crt_invalid_parameter_noinfo_00bf6713();
            void* widget = at<void*>(screen, 0xBC);
            host.widget_vcall_88(widget, node_frame(screen, first, host), 0, 1.0f);
            return;
        }

        std::uint8_t* list = base + 0x3E0;
        void* node = at<void*>(at<void*>(screen, 0x3E4), 0x0);
        if (!(at<std::uint32_t>(screen, 0x2C4) < n)) at<std::uint32_t>(screen, 0x2C4) = n - 1;
        for (std::int32_t step = 0; step < at<std::int32_t>(screen, 0x2C4); ++step) {
            list_check_node(list, node, host);
            node = at<void*>(node, 0x0);
        }
        std::int32_t frames[4];
        at<std::int32_t>(screen, 0x68) = at<std::int32_t>(screen, 0x324);
        list_check_node(list, node, host);
        {
            const std::int32_t frame = node_frame(screen, node, host);
            frames[at<std::int32_t>(screen, 0x324)] = frame;
        }
        list_check_node(list, node, host);
        if (at<std::uint8_t>(node, 0x14) != 0) {
            list_check_node(list, node, host);
            at<std::uint32_t>(screen, 0x438 + 4 * at<std::int32_t>(screen, 0x324)) =
                at<std::uint16_t>(at<void*>(node, 0x18), 0x174);
            list_check_node(list, node, host);
            place_selection_marker(screen, at<void*>(node, 0x18), host);
            host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 1);
        } else {
            at<std::uint32_t>(screen, 0x438 + 4 * at<std::int32_t>(screen, 0x324)) = 0xFFFFFFFFu;
            host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 0);
        }

        std::int32_t count = 1;
        std::int32_t index = at<std::int32_t>(screen, 0x324) - 1;
        if (index >= 0) {
            count = index + 2;
            do {
                void* begin = at<void*>(at<void*>(list, 0x4), 0x0);
                if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
                if (node == begin) node = at<void*>(list, 0x4);
                if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
                node = at<void*>(node, 0x4);
                list_check_node(list, node, host);
                at<std::int32_t>(screen, 0x68) = index;
                list_check_node(list, node, host);
                frames[index] = node_frame(screen, node, host);
                list_check_node(list, node, host);
                std::uint32_t& id = at<std::uint32_t>(screen, 0x438 + 4 * index);
                if (at<std::uint8_t>(node, 0x14) != 0) {
                    list_check_node(list, node, host);
                    id = at<std::uint16_t>(at<void*>(node, 0x18), 0x174);
                } else {
                    id = 0xFFFFFFFFu;
                }
                --index;
            } while (index >= 0);
        }

        node = at<void*>(at<void*>(list, 0x4), 0x0);
        if (!(at<std::uint32_t>(screen, 0x2C4) < at<std::uint32_t>(screen, 0x3E8))) {
            at<std::uint32_t>(screen, 0x2C4) = at<std::uint32_t>(screen, 0x3E8) - 1;
        }
        for (std::int32_t step = 0; step < at<std::int32_t>(screen, 0x2C4); ++step) {
            if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            list_check_node(list, node, host);
            node = at<void*>(node, 0x0);
        }
        for (index = at<std::int32_t>(screen, 0x324) + 1; index <= 3; ++index) {
            if (!(static_cast<std::uint32_t>(index) < at<std::uint32_t>(screen, 0x3E8))) break;
            if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            list_check_node(list, node, host);
            void* head = at<void*>(screen, 0x3E4);
            node = at<void*>(node, 0x0);
            if (list != base + 0x3E0) host.crt_invalid_parameter_noinfo_00bf6713();
            if (node == head) {
                node = at<void*>(at<void*>(screen, 0x3E4), 0x0);
                list = base + 0x3E0;
            }
            at<std::int32_t>(screen, 0x68) = index;
            if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            list_check_node(list, node, host);
            frames[index] = node_frame(screen, node, host);
            list_check_node(list, node, host);
            std::uint32_t& id = at<std::uint32_t>(screen, 0x438 + 4 * index);
            if (at<std::uint8_t>(node, 0x14) != 0) {
                list_check_node(list, node, host);
                id = at<std::uint16_t>(at<void*>(node, 0x18), 0x174);
            } else {
                id = 0xFFFFFFFFu;
            }
            ++count;
        }

        for (std::int32_t k = 0; k < count; ++k) {
            host.widget_vcall_88(at<void*>(screen, 0xBC + 4 * k), static_cast<std::uint16_t>(frames[k]), 0, 1.0f);
            host.widget_vcall_4c_set_alpha(at<void*>(screen, 0xBC + 4 * k),
                                           at<std::int32_t>(screen, 0x438 + 4 * k) > 0 ? 1.0f : dim);
            if (at<std::uint8_t>(screen, 0x6C + k) != 0) {
                host.widget_vcall_4c_set_alpha(at<void*>(screen, 0xBC + 4 * k), at<float>(screen, 0x1E0));
            }
        }
        return;
    }

    if (at<std::uint32_t>(screen, 0x1F4) == 0) return;
    void* const iterator = base + 0x300;
    auto check_iterator = [&]() {
        if (at<void*>(iterator, 0x0) == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
        if (at<void*>(iterator, 0x4) == at<void*>(at<void*>(iterator, 0x0), 0x4)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    check_iterator();
    bool group_path = host.entity_vcall_5c_is_kind(at<void*>(at<void*>(iterator, 0x4), 0x8), 0x45);
    if (!group_path) group_path = host.entity_vcall_5c_is_kind(*host.support_unit_iterator_value_00658a60(iterator), 9);

    if (!group_path) {
        at<std::uint32_t>(screen, 0x298) = 0;
        for (std::size_t k = 0; k < 4; ++k) host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC + 4 * k), 1);
        for (std::int32_t k = 0;; ++k) {
            check_iterator();
            std::uint8_t* unit = at<std::uint8_t*>(at<void*>(iterator, 0x4), 0x8);
            std::uint8_t* first = at<std::uint8_t*>(unit, 0x794);
            const std::int32_t size =
                first == nullptr ? 0 : static_cast<std::int32_t>(at<std::uint8_t*>(unit, 0x798) - first) / 0x4C;
            if (k >= size) return;
            check_iterator();
            alignas(4) std::uint8_t entry[0x4C];
            host.unit_slot_entry_copy_00846fd0(at<void*>(at<void*>(iterator, 0x4), 0x8), entry,
                                               static_cast<std::uint32_t>(k));
            if (at<std::uint32_t>(screen, 0x5C) == 0 && k == at<std::int32_t>(screen, 0x324)) {
                void* entry_unit = at<void*>(entry, 0x30);
                if (entry_unit != nullptr) {
                    host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 1);
                    place_selection_marker(screen, entry_unit, host);
                } else {
                    host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 0);
                }
            }
            void* slot = at<void*>(screen, 0xBC + 4 * k);
            if (at<std::uint32_t>(entry, 0x4) != 0) {
                host.widget_vcall_88(slot, static_cast<std::int32_t>(host.hud_unit_icon_frame_00652ba0(screen, entry)),
                                     0, 1.0f);
                const std::uint32_t state = at<std::uint32_t>(entry, 0x4);
                host.widget_vcall_4c_set_alpha(at<void*>(screen, 0xBC + 4 * k),
                                               state == 1 || state == 2 ? f32_bits(0x3F000000u) : 1.0f);
                at<std::uint32_t>(screen, 0x298) += 1;
            } else {
                host.widget_vcall_34_set_visible(slot, 0);
            }
            host.unit_slot_entry_destroy_0065bb00(entry);
        }
    }

    at<std::uint32_t>(screen, 0x298) = 0;
    check_iterator();
    std::uint8_t* group = nullptr;
    if (host.entity_vcall_5c_is_kind(at<void*>(at<void*>(iterator, 0x4), 0x8), 0x45)) {
        std::uint8_t* value = static_cast<std::uint8_t*>(*host.support_unit_iterator_value_00658a60(iterator));
        if (value != nullptr) group = value + 0x72C;
    } else {
        std::uint8_t* value = static_cast<std::uint8_t*>(*host.support_unit_iterator_value_00658a60(iterator));
        if (value != nullptr) group = value + 0x1188;
    }
    for (std::int32_t i = 0;; ++i) {
        void* lock = at<void*>(group, 0xC);
        if (lock != nullptr) {
            host.enter_critical_section_00ce2218(lock);
            at<std::int32_t>(lock, 0x18) += 1;
        }
        const std::int32_t records = at<std::int32_t>(group, 0x50);
        if (lock != nullptr) {
            at<std::int32_t>(lock, 0x18) += -1;
            host.leave_critical_section_00ce2210(lock);
        }
        if (i >= records) return;
        std::uint8_t* record = at<std::uint8_t*>(group, 0x4C) + 0x58 * i;
        if (at<std::uint32_t>(screen, 0x5C) == 0 && i == at<std::int32_t>(screen, 0x324)) {
            void* observed = at<void*>(record, 0x28);
            const std::int32_t state = at<std::int32_t>(record, 0x2C);
            if ((observed != nullptr && (state == 3 || state == 4)) || at<std::int32_t>(record, 0x2C) == 2 ||
                at<std::int32_t>(record, 0x2C) == 5) {
                if (observed != nullptr) {
                    host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 1);
                    place_selection_marker(screen, observed, host);
                } else {
                    host.widget_vcall_34_set_visible(at<void*>(screen, 0x128), 0);
                }
            }
        }
        void* slot = at<void*>(screen, 0xBC + 4 * i);
        if (at<std::int32_t>(record, 0x2C) == 1) {
            host.widget_vcall_34_set_visible(slot, 0);
            continue;
        }
        host.widget_vcall_34_set_visible(slot, 1);
        host.widget_vcall_88(at<void*>(screen, 0xBC + 4 * i),
                             static_cast<std::int32_t>(host.hud_support_record_icon_frame_00658620(screen, record)), 0,
                             1.0f);
        const std::int32_t state = at<std::int32_t>(record, 0x2C);
        float alpha;
        if (state == 2) {
            alpha = f32_bits(0x3EB33333u);  // 00CF6560, 0.35f
        } else if (state == 1 || state == 5) {
            alpha = at<float>(screen, 0x1E0);
        } else if (host.air_ops_slot_is_out_00658530(record) &&
                   reinterpret_cast<std::uintptr_t>(host.unit_vcall_174_command_class(at<void*>(record, 0x28))) ==
                       0x00E08FA0u) {
            alpha = at<float>(screen, 0x1E0);
        } else {
            alpha = 1.0f;
        }
        host.widget_vcall_4c_set_alpha(at<void*>(screen, 0xBC + 4 * i), alpha);
    }
}

// ---------------------------------------------------------------------------
// 006B9450 BSP_AchievementTable_Load (constructor)
// __fastcall(ECX = registry), RET; EAX = registry. EBP frame aligned to 8,
// SUB ESP 5A4h, EH handler 00C80FD7 (states 1..1Dh own the map, the Lua
// state and the Lua objects; noted, not modelled). Image facts (asm
// 006B9450..006B9F4D), frame offsets from the body ESP:
// - +4h/+8h/+0Ch = 0; the map +10h: head = 006B8C30, byte head+69h = 1,
//   head links to itself, size +18h = 0; then the inlined clear: 006B9120
//   on the root, head links reset, size 0. The name vector +0h..+0Ch is
//   erased whole (00BF6713 when first > last, twice; 004954F0 with the
//   result at +40h); +1Ch = 0.
// - Lua state at +E8h: 00B66BD0, 00B6A020(41h), 00B69D40(path, 0) with
//   path = "Scripts\datatables\Achievements.lua" (temporary at +18h),
//   globals at +D4h (00B67980), table +70h = globals."Achievements"
//   (00B67800), globals destroyed; key +5Ch and value +48h constructed;
//   00B67080(table, key, value).
// - While 00B66420(table, key) is false: for a string key (00B660A0):
//   record = 0050FC30(registry+10h, NativeString(key)); the key string is
//   appended to the vector +0h (00450540); then from the value, each through
//   a temporary at +20h destroyed after: Description -> record+0Ch,
//   GUITexture -> +14h, Name -> +0h, Sound -> +1Ch (inlined NativeString
//   assign from the C string, see assign_c_string), ID -> +8h (00B66290),
//   Multi -> byte +24h (00B66250), Score -> +3Ch (00B66380 default 0),
//   XLastAchievementID -> +40h (default -1). Params at +C0h, element +ACh
//   constructed; when Params is not nil, for i = 1..21: value.Params (+84h)
//   [i] (+20h) assigned to the element, both temporaries destroyed; a
//   non-nil element pushes its integer (default 0) onto record+44h
//   (00442190); a nil first element pushes 0. registry+1Ch += 1.
//   Unlock.Type (+20h, +84h) -> record+28h. Unlock (+84h).Index (+98h),
//   +84h destroyed; for i = 1.. while Index[i] (+20h) is not nil: its
//   integer is appended to the list record+30h (004857F0 before the head,
//   0048FC80(1), links). +98h, +ACh, +C0h destroyed.
//   Then 00B67190(table, key, value) and the loop test again.
// - Value, key and table destroyed; 00B669A0 on the Lua state.
// The NativeString temporaries of the key release the data and length read
// before the call that uses them (the helper re-reads them after; the
// callees take the string as const).
// ---------------------------------------------------------------------------
namespace {

// The inlined NativeString assign from a C string (len = strlen or 0 for a
// null text): a new length reallocates through 00BD1120 (length + 1 bytes,
// kind 1) and terminates, a zero length releases; then length bytes are
// copied when data is set.
void assign_c_string(void* string, const char* text, Cc13SweepB7Host& host) {
    const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0;
    Cc13SweepBNativeString& target = *static_cast<Cc13SweepBNativeString*>(string);
    if (length != target.length) {
        if (length == 0) {
            if (target.data != nullptr) {
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), target.data,
                                                      target.length + 1, 1);
            }
            target.data = nullptr;
            target.length = 0;
        } else {
            char* buffer = static_cast<char*>(
                host.sized_pool_allocate_block_00bd1120(host.sized_pool_get_00419cc0(), length + 1, 1));
            if (target.data != nullptr) {
                host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), target.data,
                                                      target.length + 1, 1);
            }
            target.data = buffer;
            target.length = length;
            buffer[length] = 0;
        }
    }
    if (target.data != nullptr) host.crt_memcpy_00bf7680(target.data, text, target.length);
}

}  // namespace

void* cc13_sweepB7_achievement_table_load_006b9450(void* registry, Cc13SweepB7Host& host) {
    std::uint8_t* const map = static_cast<std::uint8_t*>(registry) + 0x10;
    at<std::uint32_t>(registry, 0x4) = 0;
    at<std::uint32_t>(registry, 0x8) = 0;
    at<std::uint32_t>(registry, 0xC) = 0;
    at<void*>(map, 0x4) = host.award_map_buy_head_node_006b8c30(map);
    at<std::uint8_t>(at<void*>(map, 0x4), 0x69) = 1;
    at<void*>(at<void*>(map, 0x4), 0x4) = at<void*>(map, 0x4);
    at<void*>(at<void*>(map, 0x4), 0x0) = at<void*>(map, 0x4);
    at<void*>(at<void*>(map, 0x4), 0x8) = at<void*>(map, 0x4);
    at<std::uint32_t>(map, 0x8) = 0;
    host.award_map_erase_subtree_006b9120(map, at<void*>(at<void*>(map, 0x4), 0x4));
    at<void*>(at<void*>(map, 0x4), 0x4) = at<void*>(map, 0x4);
    at<std::uint32_t>(map, 0x8) = 0;
    at<void*>(at<void*>(map, 0x4), 0x0) = at<void*>(map, 0x4);
    at<void*>(at<void*>(map, 0x4), 0x8) = at<void*>(map, 0x4);

    void* const last = at<void*>(registry, 0x8);
    if (at<std::uint8_t*>(registry, 0x4) > static_cast<std::uint8_t*>(last)) host.crt_invalid_parameter_noinfo_00bf6713();
    void* const first = at<void*>(registry, 0x4);
    if (static_cast<std::uint8_t*>(first) > at<std::uint8_t*>(registry, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
    std::uint32_t erase_result[2];
    host.string_vector_erase_range_004954f0(registry, erase_result, registry, first, registry, last);
    at<std::uint32_t>(registry, 0x1C) = 0;

    // Frame locals: the Lua state owner fills the frame from +E8h to the EH
    // record (4CCh bytes); each LuaObject is 14h bytes.
    alignas(8) std::uint8_t state[0x4CC];
    alignas(4) std::uint8_t globals[0x14], table[0x14], key[0x14], value[0x14], temp[0x14], field[0x14],
        index_list[0x14], element[0x14], params[0x14];
    host.lua_state_owner_construct_00b66bd0(state);
    host.lua_state_owner_open_00b6a020(state, 0x41);
    with_c_string("Scripts\\datatables\\Achievements.lua", 0x23, host,
                  [&](Cc13SweepBNativeString* path) { host.lua_state_owner_run_script_00b69d40(state, path, 0); });
    host.lua_object_get_field_00b67800(host.lua_globals_00b67980(state, globals), table, "Achievements");
    host.lua_object_destroy_00b67700(globals);
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(value);
    host.lua_object_iterate_first_00b67080(table, key, value);

    auto read_string = [&](const char* name, void* destination) {
        assign_c_string(destination,
                        host.lua_object_get_string_00b662b0(host.lua_object_get_field_00b67800(value, temp, name)),
                        host);
        host.lua_object_destroy_00b67700(temp);
    };

    while (!host.lua_object_is_unbound_00b66420(table, key)) {
        if (host.lua_object_is_string_00b660a0(key)) {
            const char* name = host.lua_object_get_string_00b662b0(key);
            std::uint8_t* record = nullptr;
            with_c_string(name, static_cast<std::uint32_t>(std::strlen(name)), host, [&](Cc13SweepBNativeString* text) {
                record = static_cast<std::uint8_t*>(host.award_table_find_record_0050fc30(map, text));
            });
            const char* again = host.lua_object_get_string_00b662b0(key);
            with_c_string(again, static_cast<std::uint32_t>(std::strlen(again)), host,
                          [&](Cc13SweepBNativeString* text) { host.string_vector_append_00450540(registry, text); });

            read_string("Description", record + 0xC);
            read_string("GUITexture", record + 0x14);
            read_string("Name", record + 0x0);
            read_string("Sound", record + 0x1C);
            at<std::int32_t>(record, 0x8) =
                host.lua_object_get_integer_00b66290(host.lua_object_get_field_00b67800(value, temp, "ID"));
            host.lua_object_destroy_00b67700(temp);
            at<std::uint8_t>(record, 0x24) =
                host.lua_object_get_boolean_00b66250(host.lua_object_get_field_00b67800(value, temp, "Multi"));
            host.lua_object_destroy_00b67700(temp);
            at<std::int32_t>(record, 0x3C) = host.lua_object_get_integer_or_default_00b66380(
                host.lua_object_get_field_00b67800(value, temp, "Score"), 0);
            host.lua_object_destroy_00b67700(temp);
            at<std::int32_t>(record, 0x40) = host.lua_object_get_integer_or_default_00b66380(
                host.lua_object_get_field_00b67800(value, temp, "XLastAchievementID"), -1);
            host.lua_object_destroy_00b67700(temp);

            host.lua_object_get_field_00b67800(value, params, "Params");
            host.lua_object_construct_00b65f50(element);
            if (!host.lua_object_is_nil_00b65fb0(params)) {
                for (std::int32_t i = 1; i < 0x16; ++i) {
                    host.lua_object_get_field_00b67800(value, field, "Params");
                    host.lua_object_assign_00b67690(element, host.lua_object_get_by_index_00b67720(field, temp, i));
                    host.lua_object_destroy_00b67700(temp);
                    host.lua_object_destroy_00b67700(field);
                    std::int32_t number;
                    if (!host.lua_object_is_nil_00b65fb0(element)) {
                        number = host.lua_object_get_integer_or_default_00b66380(element, 0);
                        host.dword_vector_push_back_00442190(record + 0x44, &number);
                    } else if (i == 1) {
                        number = 0;
                        host.dword_vector_push_back_00442190(record + 0x44, &number);
                    }
                }
            }
            at<std::int32_t>(registry, 0x1C) += 1;

            assign_c_string(record + 0x28,
                            host.lua_object_get_string_00b662b0(host.lua_object_get_field_00b67800(
                                host.lua_object_get_field_00b67800(value, temp, "Unlock"), field, "Type")),
                            host);
            host.lua_object_destroy_00b67700(field);
            host.lua_object_destroy_00b67700(temp);

            host.lua_object_get_field_00b67800(host.lua_object_get_field_00b67800(value, field, "Unlock"), index_list,
                                               "Index");
            host.lua_object_destroy_00b67700(field);
            std::int32_t index = 1;
            bool present = !host.lua_object_is_nil_00b65fb0(host.lua_object_get_by_index_00b67720(index_list, temp, 1));
            host.lua_object_destroy_00b67700(temp);
            std::uint8_t* const list = record + 0x30;
            while (present) {
                const std::int32_t number =
                    host.lua_object_get_integer_00b66290(host.lua_object_get_by_index_00b67720(index_list, temp, index));
                void* head = at<void*>(list, 0x4);
                void* node = host.int_list_buy_node_004857f0(list, head, at<void*>(head, 0x4), &number);
                host.int_list_incsize_0048fc80(list, 1);
                at<void*>(head, 0x4) = node;
                at<void*>(at<void*>(node, 0x4), 0x0) = node;
                host.lua_object_destroy_00b67700(temp);
                ++index;
                present = !host.lua_object_is_nil_00b65fb0(host.lua_object_get_by_index_00b67720(index_list, temp, index));
                host.lua_object_destroy_00b67700(temp);
            }
            host.lua_object_destroy_00b67700(index_list);
            host.lua_object_destroy_00b67700(element);
            host.lua_object_destroy_00b67700(params);
        }
        host.lua_object_iterate_next_00b67190(table, key, value);
    }

    host.lua_object_destroy_00b67700(value);
    host.lua_object_destroy_00b67700(key);
    host.lua_object_destroy_00b67700(table);
    host.lua_state_owner_close_00b669a0(state);
    return registry;
}

// ---------------------------------------------------------------------------
// 0053F2D0 BSP_HudFreeCameraScreen_UpdateEffects
// __thiscall(ECX = block, float dt, camera), RET 8; SUB ESP F8h, EH handler
// 00C6C9B8 (state 0 owns the hit record at +A4h). Image facts (asm
// 0053F2D0..0053FEF0). The block: types vector +8h/+0Ch (30h-byte entries:
// +4h gravity, +10h damage, +14h radius, +28h impact definition, +2Ch
// splash definition), burst list +14h (head +18h, size +1Ch); a burst node:
// +0h/+4h links, +8h entry (0053DCE0), +0Ch old position, +18h position,
// +24h velocity, +30h timer, +34h trail effect, +38h type index.
// Per node (the checked list iterator tests, 00BF6713, at every node use;
// the checked index into the types vector at every type use, as the image):
// - 00470470 on the hit record; next = node->next; timer -= dt.
// - 0 > timer (COMISS, JBE keeps it alive; NaN stays alive): the trail gets
//   byte +9h = 1 and 00867B10; when the type has an impact definition, the
//   impact is spawned at the hit record's +8h..+10h (identity matrix with that
//   translation, 00868420 on [game+19ECh], the definition passed by value
//   with its reference added, the returned object's byte +9h = 1 after the
//   out reference is dropped) and the blast 0084BAD0(position, &radius,
//   &damage, 0, 0, 0) runs; the node is unlinked, 0053DCE0, freed and the
//   size decremented (unless it is the head).
// - Alive: step = velocity * dt (float stores), position += step, vy +=
//   gravity * dt, old position <- position (the previous old saved first),
//   the trail placed with the velocity as row 2 (0085DC80) and the position
//   as row 3 (0053D9C0). 009043A0([game+19CCh], saved old, position, hit,
//   0, 0): a hit selects the root parent (+3Ch chain) of the hit unit for the
//   debug inspector when it is +5Ch(1), spawns the impact at the hit point
//   and blasts there; else 0078D6C0([game+19F0h], saved old, position, out)
//   true spawns the splash definition at out and blasts there; either hit
//   stops the trail and erases the node (0053EB10); no hit keeps it.
// - 004704B0 on the hit record; continue with the saved next.
// - At the end (node == head): 0053EE00(block, dt, camera).
// ---------------------------------------------------------------------------
namespace {

struct BurstContext {
    void* block;
    std::uint8_t* list;
    void* container;
    void* node;
    Cc13SweepB7Host* host;

    void check() const {
        if (node == at<void*>(container, 0x4)) host->crt_invalid_parameter_noinfo_00bf6713();
    }
    // The checked index of the node's type into the vector +8h; returns the entry.
    std::uint8_t* type() const {
        std::uint8_t* begin = at<std::uint8_t*>(block, 0x8);
        const std::uint32_t index = at<std::uint32_t>(node, 0x38);
        if (begin == nullptr ||
            !(index < static_cast<std::uint32_t>(
                          static_cast<std::int32_t>(at<std::uint8_t*>(block, 0xC) - begin) / 0x30))) {
            host->crt_invalid_parameter_noinfo_00bf6713();
        }
        return at<std::uint8_t*>(block, 0x8) + 0x30 * index;
    }
};

// The identity matrix with a translation, as the image builds it on the
// stack before each 00868420.
void identity_with_translation(float* m, const float* translation) {
    const float one = f32_bits(0x3F800000u);  // 00D7A24C
    for (int k = 0; k < 16; ++k) m[k] = 0.0f;
    m[0] = one;
    m[5] = one;
    m[10] = one;
    m[15] = one;
    m[12] = translation[0];
    m[13] = translation[1];
    m[14] = translation[2];
}

// 00868420 with the type's definition at type+definition_offset (reference
// added inline, owned by the callee), the out reference dropped, then byte
// +9h of the created object set.
void spawn_point_effect(const BurstContext& c, std::size_t definition_offset, const float* matrix) {
    Cc13SweepB7Host& host = *c.host;
    void* system = at<void*>(*static_cast<void**>(host.image_global(0x00E188A8)), 0x19EC);
    c.check();
    void* definition = nullptr;
    void* source = at<void*>(c.type(), definition_offset);
    if (source != nullptr) {
        definition = source;
        host.interlocked_increment_00ce221c(&at<std::int32_t>(source, 0x4));
    }
    void* out_slot = nullptr;
    void* object = *static_cast<void**>(
        host.point_effect_create_from_matrix_00868420(&out_slot, system, definition, matrix, 0, 0));
    if (out_slot != nullptr) {
        void* held = out_slot;
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(held, 0x4)) == 0) {
            host.refcounted_vcall_00_destroy(held);
        }
        out_slot = nullptr;
    }
    if (object != nullptr) at<std::uint8_t>(object, 0x9) = 1;
}

// The blast after a spawn: the type is re-checked for the damage and the
// radius pointers, then 0084BAD0(centre, &radius, &damage, 0, 0, 0).
void blast(const BurstContext& c, const float* centre) {
    c.check();
    std::uint8_t* damage_type = c.type();
    c.check();
    std::uint8_t* radius_type = c.type();
    c.check();
    c.host->explosion_apply_radial_damage_0084bad0(centre, &at<float>(radius_type, 0x14),
                                                   &at<float>(damage_type, 0x10), 0, nullptr, nullptr);
}

void stop_trail(const BurstContext& c) {
    c.check();
    if (at<void*>(c.node, 0x34) != nullptr) {
        c.check();
        at<std::uint8_t>(at<void*>(c.node, 0x34), 0x9) = 1;
        c.check();
        c.host->point_effect_stop_children_00867b10(at<void*>(c.node, 0x34));
    }
}

}  // namespace

void cc13_sweepB7_free_camera_update_bursts_0053f2d0(void* block, float dt, const void* camera,
                                                     Cc13SweepB7Host& host) {
    std::uint8_t* const list = static_cast<std::uint8_t*>(block) + 0x14;
    void* container = list;
    void* node = at<void*>(at<void*>(list, 0x4), 0x0);
    alignas(4) std::uint8_t hit[0x54];
    for (;;) {
        void* const head = at<void*>(list, 0x4);
        if (container == nullptr || container != list) host.crt_invalid_parameter_noinfo_00bf6713();
        if (node == head) {
            cc13_sweepB4_free_camera_effect_update_0053ee00(block, dt, camera, host);
            return;
        }
        host.hit_record_reset_00470470(hit);
        BurstContext c{block, list, container, node, &host};
        if (container == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
        c.check();
        void* const next_container = container;
        const bool at_end = node == at<void*>(container, 0x4);
        void* const next = at<void*>(node, 0x0);
        if (at_end) host.crt_invalid_parameter_noinfo_00bf6713();
        at<float>(node, 0x30) = at<float>(node, 0x30) - dt;
        c.check();

        if (0.0f > at<float>(node, 0x30)) {
            stop_trail(c);
            c.check();
            if (at<void*>(c.type(), 0x28) != nullptr) {
                float matrix[16];
                identity_with_translation(matrix, &at<float>(hit, 0x8));
                spawn_point_effect(c, 0x28, matrix);
                blast(c, &at<float>(node, 0x18));
            }
            c.check();
            if (node != at<void*>(block, 0x18)) {
                at<void*>(at<void*>(node, 0x4), 0x0) = at<void*>(node, 0x0);
                at<void*>(at<void*>(node, 0x0), 0x4) = at<void*>(node, 0x4);
                host.free_camera_effect_entry_destruct_0053dce0(static_cast<std::uint8_t*>(node) + 0x8);
                host.crt_operator_delete_00bf65ac(node);
                at<std::int32_t>(list, 0x8) += -1;
            }
        } else {
            c.check();
            c.check();
            const float step_x = dt * at<float>(node, 0x24);
            const float step_y = at<float>(node, 0x28) * dt;
            const float step_z = dt * at<float>(node, 0x2C);
            at<float>(node, 0x18) = step_x + at<float>(node, 0x18);
            at<float>(node, 0x1C) = at<float>(node, 0x1C) + step_y;
            at<float>(node, 0x20) = step_z + at<float>(node, 0x20);
            if (node == at<void*>(container, 0x4)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
                c.check();
            }
            at<float>(node, 0x28) =
                static_cast<float>(static_cast<double>(at<float>(c.type(), 0x4)) * dt + at<float>(node, 0x28));
            c.check();
            float old_position[3] = {at<float>(node, 0xC), at<float>(node, 0x10), at<float>(node, 0x14)};
            c.check();
            c.check();
            at<float>(node, 0xC) = at<float>(node, 0x18);
            at<float>(node, 0x10) = at<float>(node, 0x1C);
            at<float>(node, 0x14) = at<float>(node, 0x20);
            c.check();
            if (at<void*>(node, 0x34) != nullptr) {
                float matrix[16];
                const float origin[3] = {0.0f, 0.0f, 0.0f};
                identity_with_translation(matrix, origin);
                c.check();
                matrix[8] = at<float>(node, 0x24);
                matrix[9] = at<float>(node, 0x28);
                matrix[10] = at<float>(node, 0x2C);
                host.matrix_orthonormalize_rows_0085dc80(matrix);
                c.check();
                c.check();
                matrix[12] = at<float>(node, 0x18);
                matrix[13] = at<float>(node, 0x1C);
                matrix[14] = at<float>(node, 0x20);
                host.point_effect_set_world_matrix_0053d9c0(at<void*>(node, 0x34), matrix);
            }
            c.check();
            std::uint8_t* const game = *static_cast<std::uint8_t**>(host.image_global(0x00E188A8));
            if (host.world_ray_test_009043a0(at<void*>(game, 0x19CC), old_position, &at<float>(node, 0x18), hit,
                                             nullptr, 0)) {
                void* unit = at<void*>(hit, 0x0);
                if (unit != nullptr) {
                    while (at<void*>(unit, 0x3C) != nullptr) unit = at<void*>(unit, 0x3C);
                    if (host.entity_vcall_5c_is_kind(unit, 1)) {
                        host.unit_debug_inspector_set_target_006fb6a0(host.singleton_00e19aac_get_0051f5e0(), unit);
                    }
                }
                c.check();
                if (at<void*>(c.type(), 0x28) != nullptr) {
                    float matrix[16];
                    identity_with_translation(matrix, &at<float>(hit, 0x8));
                    spawn_point_effect(c, 0x28, matrix);
                    c.check();
                    std::uint8_t* damage_type = c.type();
                    c.check();
                    std::uint8_t* radius_type = c.type();
                    const float point[3] = {at<float>(hit, 0x8), at<float>(hit, 0xC), at<float>(hit, 0x10)};
                    host.explosion_apply_radial_damage_0084bad0(point, &at<float>(radius_type, 0x14),
                                                                &at<float>(damage_type, 0x10), 0, nullptr, nullptr);
                }
                stop_trail(c);
                std::uint32_t result[2];
                host.effect_list_erase_0053eb10(list, result, container, node);
            } else {
                c.check();
                const float to_copy[3] = {at<float>(node, 0x18), at<float>(node, 0x1C), at<float>(node, 0x20)};
                const float from_copy[3] = {old_position[0], old_position[1], old_position[2]};
                float out[3];
                if (host.segment_test_0078d6c0(at<void*>(game, 0x19F0), from_copy, to_copy, out)) {
                    c.check();
                    if (at<void*>(c.type(), 0x2C) != nullptr) {
                        float matrix[16];
                        identity_with_translation(matrix, out);
                        spawn_point_effect(c, 0x2C, matrix);
                        c.check();
                        std::uint8_t* damage_type = c.type();
                        c.check();
                        std::uint8_t* radius_type = c.type();
                        host.explosion_apply_radial_damage_0084bad0(out, &at<float>(radius_type, 0x14),
                                                                    &at<float>(damage_type, 0x10), 0, nullptr,
                                                                    nullptr);
                    }
                    stop_trail(c);
                    std::uint32_t result[2];
                    host.effect_list_erase_0053eb10(list, result, container, node);
                }
            }
        }
        host.hit_record_destroy_004704b0(hit);
        container = next_container;
        node = next;
    }
}

}  // namespace bsp
