// cc13_pkt642: the HUD unit-marker builder 00642040 and its widget writers
// 0063C620 / 0063D1E0 (packet cc13-pkt642, docs/CC13_PKT642.md). Names are
// hypotheses; see the header for the hooks.
//
// x87 note (as cc13_sweepB..B6): float expressions follow the image's
// operation order with a float store wherever the image stores to a float
// slot; intermediate x87 values are kept in double, and double constants are
// applied in double. EH frames are noted, not modelled. Register-register x87
// operand directions were read from the bytes (DE E1 = FSUBRP ST1: ST0 - ST1;
// DE E9 = FSUBP ST1: ST1 - ST0; DE C1 = FADDP).

#include "bsp/cc13_pkt642.hpp"

#include <cstring>

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
    return cc13_sweepB_f32(bits);
}

// Constants read from the image (pe_const_read; bit patterns kept exact).
const float kSizeA_X = f32_bits(0x3D11105Eu);              // 00CF576C  0.035416
const float kSizeA_Y = f32_bits(0x3D9F4A12u);              // 00CF5768  0.077778
const double kSizeB_X = f64_bits(0x3FAB3311A0000000ull);   // 00CF5840  0.053124 (double)
const double kSizeB_Y = f64_bits(0x3FBDDDE360000000ull);   // 00CF5838  0.116667 (double)
const float kCornerSizeX = f32_bits(0x3C91105Eu);          // 00CF5834  0.017708
const float kCornerSizeY = f32_bits(0x3D1F4A12u);          // 00CF5830  0.038889
const float kMinusOne = f32_bits(0xBF800000u);             // 00D7A260  -1.0f
const double kHalf = f64_bits(0x3FE0000000000000ull);      // 00D7A280  0.5
const double k0_04 = f64_bits(0x3FA47AE140000000ull);      // 00CE4D68  0.04 (float-rounded)
const double k0_01 = f64_bits(0x3F847AE140000000ull);      // 00D7A358  0.01 (float-rounded)
const double k0_05 = f64_bits(0x3FA99999A0000000ull);      // 00D7A270  0.05 (float-rounded)
const float kOne = f32_bits(0x3F800000u);                  // 00D7A24C  1.0f
const double k0_29 = f64_bits(0x3FD28F5C20000000ull);      // 00CE6808  0.29 (float-rounded)
const float kAngleBase = f32_bits(0xC016CBE4u);            // 00CF5814  -3*pi/4
const double kHalfPi = f64_bits(0x3FF921FB60000000ull);    // 00CE3830  pi/2 (float-rounded)
const float k0_9f = f32_bits(0x3F666666u);                 // 00CE3860  0.9f
const float k0_7f = f32_bits(0x3F333333u);                 // 00CE3E18  0.7f
const double k960 = f64_bits(0x408E000000000000ull);       // 00CEC380  960.0
const float kMinusThree = f32_bits(0xC0400000u);           // 00CE3D50  -3.0f
const double k1_6 = f64_bits(0x3FF99999A0000000ull);       // 00CE3D48  1.6 (float-rounded)
const float kMinus23 = f32_bits(0xC1B80000u);              // 00CF5810  -23.0f
const float k0_5f = f32_bits(0x3F000000u);                 // 00CE3800  0.5f
const float kGroupSizeX = f32_bits(0x3D59988Du);           // 00CF5774  0.053124f
const float kGroupSizeY = f32_bits(0x3DEEEF1Bu);           // 00CF5770  0.116667f
const double k0_02 = f64_bits(0x3F947AE140000000ull);      // 00D7A2F8  0.02 (float-rounded)
const double k0_03 = f64_bits(0x3F9EB851E0000000ull);      // 00CEB690  0.03 (float-rounded)
const float k0_4f = f32_bits(0x3ECCCCCDu);                 // 00CE7804  0.4f
const double kArcA = f64_bits(0x400363F141205BC0ull);      // 00CF58A0  2.4238
const double kPi = f64_bits(0x400921FB60000000ull);        // 00CE3D28  pi (float-rounded)
const double kMinusPi = f64_bits(0xC00921FB60000000ull);   // 00CE3D18  -pi (float-rounded)
const double kArcD = f64_bits(0xC003350B0F27BB30ull);      // 00CF5898  -2.4009
const double kArcH = f64_bits(0x3FE6F8287B7E9100ull);      // 00CF5890  0.7177927...
const double k0_06 = f64_bits(0x3FAEB851E0000000ull);      // 00CF0AC0  0.06 (float-rounded)
const double kArcDiv1 = f64_bits(0x3FF755F4E0000000ull);   // 00CF5888  1.458485
const double kArcF = f64_bits(0x3FE7C0EBEDFA43FEull);      // 00CF5880  0.7423
const double kArcDiv2 = f64_bits(0x3FFAE76C80000000ull);   // 00CF5878  1.6815
const double k0_09 = f64_bits(0x3FB70A3D80000000ull);      // 00CF5870  0.09 (float-rounded)
const double kArcG = f64_bits(0xBFE7C0EBEDFA43FEull);      // 00CF5868  -0.7423
const float kMargin = f32_bits(0x3D50E560u);               // 00CF5860  0.051f
const float kMarginFlag = f32_bits(0x3CF5C28Fu);           // 00CE3DB8  0.03f
const double kArcDiv3 = f64_bits(0x3FF7C0EBE0000000ull);   // 00CF5858  1.4846
const double kArcDiv4 = f64_bits(0x3FFA89A020000000ull);   // 00CF5850  1.6586
const double k0_046 = f64_bits(0x3FA78D4FE0000000ull);     // 00CF5848  0.046 (float-rounded)
const float kAnchorAngle = f32_bits(0x3FC90FDBu);          // 00CE3C64  pi/2 (float)

constexpr std::uint32_t kCommandBuildingIcon = 0x00CF5818u;  // "commandbuilding_Icon" (14h)
constexpr std::uint32_t kUnitNameText = 0x00CF58CCu;         // "Unit_name_Text" (0Eh)
constexpr std::uint32_t kDistanceText = 0x00CF58BCu;         // "Distance_Text" (0Dh)
constexpr std::uint32_t kHpIcon = 0x00CF58B4u;               // "HP_Icon" (7)
constexpr std::uint32_t kHpBgIcon = 0x00CF58A8u;             // "HP_BG_Icon" (0Ah)
constexpr std::uint32_t kTypeIcon = 0x00CF57B4u;             // "type_Icon" (9)
constexpr std::uint32_t kEmptyString = 0x00CE3A0Cu;          // ""
constexpr std::uint32_t kGlobalsUnknown = 0x00CE4180u;       // "globals.unknown"
constexpr std::uint32_t kAlphaBase = 0x00F88A00u;            // float global (.bss)

void* widget(void* record, std::size_t offset) {
    return at<void*>(record, offset);
}

// The inlined native-string build from a literal: {0, null}, 0041DD40(&s,
// length, 1), then when data is non-null 00BF7680(data, literal, length + 1)
// with the length the resize left.
void build_literal(Cc13SweepBNativeString& text, std::uint32_t literal, std::uint32_t length,
                   Cc13Pkt642Host& host) {
    text.length = 0;
    text.data = nullptr;
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) {
        host.crt_memcpy_00bf7680(text.data, host.image_global(literal), text.length + 1);
    }
}

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13Pkt642Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined child lookup: literal string, 00AA7E00(parent, &s, 1), release.
void* find_child(void* parent, std::uint32_t literal, std::uint32_t length, Cc13Pkt642Host& host) {
    Cc13SweepBNativeString name;
    build_literal(name, literal, length, host);
    void* child = host.widget_find_child_00aa7e00(parent, &name, 1);
    release_string(name, host);
    return child;
}

void set_colour(void* target, float r, float g, float b, float a, Cc13Pkt642Host& host) {
    const float colour[4] = {r, g, b, a};
    host.widget_vcall_50_colour(target, colour);
}

void set_white(void* target, Cc13Pkt642Host& host) {
    set_colour(target, kOne, kOne, kOne, kOne, host);
}

bool low_byte(std::uint32_t value) {
    return (value & 0xFFu) != 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0063C620 BSP_HudMarker_EmitBoxMarker
// ---------------------------------------------------------------------------
// __thiscall(ECX = marker record), RET 38h; SEH 00C79FC0 with state 0 owning
// the by-value distance string. Frame base E = entry ESP - 68h; arguments at
// E+6Ch.. (corners, group, show, text_mode, text, unit, distance {len E+84h,
// data E+88h}, scale E+8Ch, colour E+90h..9Ch, flag E+A0h). The image reuses
// the dead argument slots E+6Ch, E+74h, E+78h and E+A0h as float temporaries;
// they are locals here. Record layout read: +4 byte, +5 byte, +8..+64h the 24
// group widgets (group g at +8 + g*10h), +68h..+80h seven widgets.
void cc13_pkt642_hud_marker_emit_box_marker_0063c620(void* record, float* corners, std::int32_t group,
                                                     std::uint32_t show, std::uint32_t text_mode,
                                                     const char* text, void* unit,
                                                     Cc13SweepBNativeString distance, float scale,
                                                     Cc13Pkt642Colour colour, std::uint32_t flag,
                                                     Cc13Pkt642Host& host) {
    // 0063C63E: E+13h = unit ? unit->+5Ch(1Ch) : 0.
    bool special = false;
    if (unit != nullptr) {
        special = host.entity_vcall_5c_is_kind(unit, 0x1c);
    }
    bool show_header = false;  // E+12h, cleared at 0063C66F
    host.widget_vcall_34_set_visible(widget(record, 0x68), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x6c), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x7c), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x70), 0);

    // 0063C69A..0063C761: six passes over the four corners; EDI walks all 24
    // group widgets once (it is not reset per pass), EBP restarts at corners.
    std::size_t slot = 8;
    for (int pass = 0; pass < 6; ++pass) {
        for (int corner = 0; corner < 4; ++corner, slot += 4) {
            void* w = widget(record, slot);
            host.widget_vcall_34_set_visible(w, 0);
            const float* size = host.widget_get_size_00aa6740(w);
            const float sx = size[0];
            const float sy = size[1];
            // UCOMISS/LAHF/TEST AH,44h: JP = not equal or unordered.
            bool resize = sx == kSizeA_X && sy == kSizeA_Y;
            if (!resize) {
                // 0063C6F1: FUCOMIP against the doubles; JP skips.
                resize = static_cast<double>(sx) == kSizeB_X && static_cast<double>(sy) == kSizeB_Y;
            }
            if (resize) {
                const float pair[2] = {kCornerSizeX, kCornerSizeY};
                host.widget_vcall_58_set_pair(w, pair);
            }
            host.hud_marker_clamp_to_margins_00638b90(&corners[corner * 2], &corners[corner * 2 + 1]);
        }
    }

    // 0063C767: the +78h text widget above the box (text_mode set) or below it.
    {
        float position[3];
        position[2] = kMinusOne;  // E+38h
        const float x = static_cast<float>((static_cast<double>(corners[4]) + corners[2]) * kHalf);
        float y;
        if (low_byte(text_mode)) {
            y = static_cast<float>(static_cast<double>(corners[1]) - k0_04);
        } else {
            y = static_cast<float>(static_cast<double>(corners[5]) + k0_01 + k0_05);
        }
        position[0] = x;
        position[1] = y;
        host.widget_set_resolved_position_00aa8240(widget(record, 0x78), position);
    }

    void* group_base = nullptr;  // EBX: &record[8 + group*10h]
    if (special) {
        // 0063C81B: the command-building icon at the anchor.
        void* holder = widget(record, 0x74);
        const float scale_pair[2] = {kOne, kOne};
        host.widget_vcall_48_set_pair(holder, scale_pair);
        host.widget_vcall_34_set_visible(holder, 1);
        const float anchor[3] = {corners[8], corners[9], 0.0f};
        host.gui_widget_set_local_position_00aa7dc0(holder, anchor);
        void* icon = find_child(holder, kCommandBuildingIcon, 0x14, host);
        host.widget_vcall_88(icon, at<std::int32_t>(unit, 0x54), 0, 1.0f);
        host.widget_get_size_00aa6740(icon);  // 0063C8F8: result unused
        group_base = &at<std::uint8_t>(record, 8 + static_cast<std::size_t>(group) * 0x10);
        for (int i = 0; i < 4; ++i) {
            host.widget_vcall_34_set_visible(at<void*>(group_base, i * 4), static_cast<std::int32_t>(show));
        }
        // 0063C927: SETNZ on [00E188D8] != unit.
        show_header = host.global_00e188d8() != unit;
    } else {
        host.widget_vcall_34_set_visible(widget(record, 0x74), 0);
        group_base = &at<std::uint8_t>(record, 8 + static_cast<std::size_t>(group) * 0x10);
        for (int i = 0; i < 4; ++i) {
            host.widget_vcall_34_set_visible(at<void*>(group_base, i * 4), 1);
        }
        const bool mode = low_byte(text_mode);
        if (!mode) {
            show_header = true;
        }
        if (at<std::uint8_t>(record, 4) == 0) {
            host.widget_vcall_34_set_visible(widget(record, 0x78), 1);
            if (!mode) {
                host.widget_vcall_34_set_visible(widget(record, 0x68), 1);
                host.widget_vcall_34_set_visible(widget(record, 0x7c), 1);
                host.widget_vcall_34_set_visible(widget(record, 0x70), 1);
                host.widget_vcall_34_set_visible(widget(record, 0x6c), 1);
            }
        } else {
            host.widget_vcall_34_set_visible(widget(record, 0x68), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x7c), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x70), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x78), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x6c), 0);
            show_header = false;
            text = nullptr;  // 0063CA01: the text argument slot is zeroed
        }
    }

    // 0063CA09.
    host.widget_vcall_34_set_visible(widget(record, 0x80), 0);
    std::int32_t frame;  // E+14h
    if (low_byte(show)) {
        frame = 1;
    } else {
        frame = at<std::uint8_t>(record, 5) != 0 ? 2 : 0;
    }

    if (!special || at<std::uint8_t>(record, 4) == 0) {
        // 0063CA48: the four corner widgets of the group.
        for (std::int32_t i = 0; i < 4; ++i) {
            void* w = at<void*>(group_base, static_cast<std::size_t>(i) * 4);
            if (at<std::uint8_t>(record, 5) != 0 && i != 0) {
                host.widget_vcall_34_set_visible(w, 0);
                continue;
            }
            host.widget_vcall_34_set_visible(w, 1);
            const float position[3] = {corners[i * 2], corners[i * 2 + 1], 0.0f};
            host.gui_widget_set_local_position_00aa7dc0(w, position);
            host.widget_vcall_88(w, frame, 0, 1.0f);
            float rotation;
            if (at<std::uint8_t>(record, 5) == 0) {
                const float base = low_byte(show) ? kAngleBase : 0.0f;  // E+18h
                // FILD i; FMUL pi/2; FADD base; FSTP float E+24h; FLD.
                rotation = static_cast<float>(static_cast<double>(i) * kHalfPi + base);
            } else {
                rotation = 0.0f;  // FLDZ
            }
            host.widget_vcall_44_set_float(w, rotation);
            if (at<std::uint8_t>(record, 4) != 0 && !low_byte(show)) {
                float alpha = *static_cast<float*>(host.image_global(kAlphaBase));  // E+78h
                if (group == 2 || group == 3 || group == 4) {
                    alpha = static_cast<float>(alpha + kHalf);
                } else if (low_byte(flag)) {
                    alpha = static_cast<float>(alpha + k0_29);
                }
                set_colour(w, kOne, kOne, kOne, alpha, host);
            } else {
                set_white(w, host);
            }
        }
    }

    // 0063CBC6: the caption text.
    if (text != nullptr) {
        // SETNZ DL: only the low byte is defined; the upper bytes of EDX are
        // left from the previous call sequence.
        host.widget_vcall_34_set_visible(widget(record, 0x78), text[0] != 0 ? 1 : 0);
        host.gui_text_set_localised_cstring_00abbe50(widget(record, 0x78), text, 1);
        set_white(widget(record, 0x78), host);
    }

    if (show_header) {
        // 0063CC24.
        bool in_range = false;  // BL
        if (host.global_00e188d8() != nullptr) {
            void* owner = at<void*>(host.global_00e198c4(), 0x48);
            in_range = host.hud_marker_is_within_display_range_00639810(owner, unit);
        }
        if (special && at<std::uint8_t>(record, 4) != 0) {
            host.widget_vcall_34_set_visible(widget(record, 0x7c), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x70), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x6c), 0);
            host.widget_vcall_34_set_visible(widget(record, 0x68), 0);
        } else {
            void* distance_text = widget(record, 0x7c);
            host.widget_vcall_34_set_visible(distance_text, 1);
            host.gui_text_set_localised_source_00abaed0(distance_text, &distance, 1);
            if (in_range) {
                set_colour(distance_text, kOne, kOne, kOne, k0_9f, host);
            } else {
                set_colour(distance_text, k0_9f, k0_9f, k0_9f, k0_7f, host);
            }

            // 0063CD19: the caption (+78h) and distance (+7Ch) anchors.
            float caption[3];  // E+18h
            host.widget_get_resolved_position_00aa6750(widget(record, 0x78), caption);
            float caption_half =  // E+78h
                static_cast<float>(at<float>(widget(record, 0x78), 0x114) / k960);
            const float moved[3] = {caption[0], caption[1], kMinusThree};  // E+24h
            caption_half = static_cast<float>(caption_half * kHalf);
            host.widget_set_resolved_position_00aa8240(distance_text, moved);
            float dist[3];  // E+30h
            host.widget_get_resolved_position_00aa6750(distance_text, dist);
            float dist_half =  // E+A0h
                static_cast<float>(at<float>(distance_text, 0x114) / k960);
            void* type_icon = widget(record, 0x70);
            host.widget_vcall_34_set_visible(type_icon, 1);
            dist_half = static_cast<float>(dist_half * kHalf);
            // 0063CDC0: unit->+140h() (no stack arguments), 0059CB90(ECX =
            // result, EDX = 0), then +70h->+88h(index, 0, 1.0f).
            void* instance = host.unit_vcall_140(unit);
            const std::int32_t icon = host.unit_hud_icon_index_0059cb90(instance, nullptr);
            host.widget_vcall_88(type_icon, icon, 0, 1.0f);

            // 0063CDEE: FCOMI caption_half vs dist_half; JBE = <= or unordered.
            const bool caption_wider = caption_half > dist_half;
            float x;  // E+6Ch
            double left;
            if (caption_wider) {
                left = static_cast<double>(caption[0]) - caption_half;  // FSUBR: E+18h - ST0, kept double (E+24h)
            } else {
                left = static_cast<double>(dist[0]) - dist_half;
            }
            {
                const float* size = host.widget_get_size_00aa6740(type_icon);
                x = static_cast<float>(left - static_cast<double>(size[0]) * kHalf);
            }
            const float* size = host.widget_get_size_00aa6740(type_icon);
            float y = static_cast<float>(static_cast<double>(size[1]) / k1_6 +
                                         (caption_wider ? caption[1] : dist[1]));  // E+74h
            const float type_position[3] = {x, y, kOne};
            host.widget_set_resolved_position_00aa8240(type_icon, type_position);
            set_white(type_icon, host);

            // 0063CECD: +6Ch to the right of the wider text.
            void* right_icon = widget(record, 0x6c);
            host.widget_vcall_34_set_visible(right_icon, 1);
            {
                const bool wider = caption_half > dist_half;  // second FCOMI, same operands
                if (wider) {
                    x = static_cast<float>(static_cast<double>(caption_half) + caption[0] + k0_01);
                    y = static_cast<float>(static_cast<double>(caption[1]) + k0_05);
                } else {
                    x = static_cast<float>(static_cast<double>(dist_half) + dist[0] + k0_01);
                    y = static_cast<float>(static_cast<double>(dist[1]) + k0_05);
                }
                const float position[3] = {x, y, kMinus23};
                host.widget_set_resolved_position_00aa8240(right_icon, position);
            }
            set_white(right_icon, host);

            // 0063CF7B: +68h at the same place, coloured and sized.
            void* bar = widget(record, 0x68);
            host.widget_vcall_34_set_visible(bar, 1);
            {
                const bool wider = caption_half > dist_half;
                float bx;  // E+78h
                float by;  // E+A0h
                if (wider) {
                    bx = static_cast<float>(static_cast<double>(caption_half) + caption[0] + k0_01);
                    by = static_cast<float>(static_cast<double>(caption[1]) + k0_05);
                } else {
                    bx = static_cast<float>(static_cast<double>(dist_half) + dist[0] + k0_01);
                    by = static_cast<float>(static_cast<double>(dist[1]) + k0_05);
                }
                const float position[3] = {bx, by, kMinus23};  // z still E+44h from 0063CEEE
                host.widget_set_resolved_position_00aa8240(bar, position);
            }
            host.widget_vcall_50_colour(bar, colour.rgba);
            const float bar_width = host.widget_get_size_00aa6740(bar)[0];
            const float* right_size = host.widget_get_size_00aa6740(right_icon);
            const float bar_height = static_cast<float>(static_cast<double>(right_size[1]) * scale);
            const float pair[2] = {bar_width, bar_height};
            host.widget_vcall_58_set_pair(bar, pair);

            // 0063D063: dimmed when unit->+5Ch(1Bh) and byte unit+720h.
            if (host.entity_vcall_5c_is_kind(unit, 0x1b) && at<std::uint8_t>(unit, 0x720) != 0) {
                set_colour(widget(record, 0x7c), k0_5f, k0_5f, k0_5f, kOne, host);
                set_colour(widget(record, 0x78), k0_5f, k0_5f, k0_5f, kOne, host);
                set_colour(widget(record, 0x70), k0_5f, k0_5f, k0_5f, kOne, host);
                set_colour(widget(record, 0x6c), k0_5f, k0_5f, k0_5f, kOne, host);
                set_colour(widget(record, 0x68), k0_5f, k0_5f, k0_5f, kOne, host);
            }
        }
    }

    // 0063D199: the callee releases the by-value distance string.
    release_string(distance, host);
}

// ---------------------------------------------------------------------------
// 0063D1E0 BSP_HudMarker_WriteUnitMarkerWidgets
// ---------------------------------------------------------------------------
// __thiscall(ECX = marker record), RET 38h; SEH 00C7A010 (state 0 owns the
// by-value distance string; states 1..7 the temporary name strings). Frame
// base E = entry ESP - 84h; arguments at E+88h.. (unit, x E+8Ch, y E+90h,
// angle E+94h, group E+98h, colour E+9Ch..A8h, name E+ACh, distance {len
// E+B0h, data E+B4h}, scale E+B8h, flag E+BCh). The image reuses the dead
// slots E+8Ch..E+BCh for children and float temporaries; they are locals here.
void cc13_pkt642_hud_marker_write_unit_marker_widgets_0063d1e0(void* record, void* unit, float x, float y,
                                                               float angle, std::int32_t group,
                                                               Cc13Pkt642Colour colour, const char* name,
                                                               Cc13SweepBNativeString distance, float scale,
                                                               std::uint32_t flag, Cc13Pkt642Host& host) {
    const float arc = angle;  // E+24h, saved before anything else
    host.hud_marker_clamp_to_margins_00638b90(&x, &y);
    float out_x = x;  // E+14h
    float out_y = y;  // E+18h
    host.widget_vcall_34_set_visible(widget(record, 0x74), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x68), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x7c), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x70), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x78), 0);
    host.widget_vcall_34_set_visible(widget(record, 0x6c), 0);
    for (std::size_t slot = 8; slot < 8 + 24 * 4; slot += 4) {
        host.widget_vcall_34_set_visible(widget(record, slot), 0);
    }
    const bool special = host.entity_vcall_5c_is_kind(unit, 0x1c);  // E+13h

    void** group_slot = &at<void*>(record, 8 + static_cast<std::size_t>(group) * 0x10);
    host.widget_vcall_34_set_visible(*group_slot, 1);
    host.widget_vcall_88(*group_slot, 1, 0, 1.0f);
    void* arrow = *group_slot;  // EDI, loaded at 0063D308 before the +80h call
    host.widget_vcall_34_set_visible(widget(record, 0x80), 1);
    angle = static_cast<float>(static_cast<double>(angle) - kHalfPi);
    const bool flagged = low_byte(flag);  // BL
    if (arrow != nullptr) {
        // 0063D32A: the null test comes after the two calls above, as in the image.
        host.widget_vcall_44_set_float(arrow, angle);
        set_white(arrow, host);
        const float position[3] = {x, y, flagged ? kMinusOne : 0.0f};
        host.gui_widget_set_local_position_00aa7dc0(arrow, position);
        if (flagged) {
            const float pair[2] = {kGroupSizeX, kGroupSizeY};
            host.widget_vcall_58_set_pair(arrow, pair);
        }
    }

    void* panel = widget(record, 0x80);
    void* name_text = find_child(panel, kUnitNameText, 0x0e, host);       // E+94h
    void* distance_text = find_child(panel, kDistanceText, 0x0d, host);   // E+1Ch
    void* hp_icon = find_child(panel, kHpIcon, 7, host);                   // E+20h
    void* hp_background = find_child(panel, kHpBgIcon, 0x0a, host);       // EBX
    void* type_icon = find_child(panel, kTypeIcon, 9, host);              // EDI

    host.widget_vcall_34_set_visible(distance_text, 1);
    {
        // 0063D646: the inlined copy of the by-value string: resize to its
        // length, then copy `length` bytes (no terminator) when the source
        // length is non-zero.
        Cc13SweepBNativeString copy{0, nullptr};
        host.native_string_resize_0041dd40(&copy, distance.length, 1);
        if (distance.length != 0) {
            host.crt_memcpy_00bf7680(copy.data, distance.data, copy.length);
        }
        host.gui_text_set_localised_source_00abaed0(distance_text, &copy, 1);
        release_string(copy, host);
    }
    if (host.global_00e188d8() != nullptr &&
        host.hud_marker_is_within_display_range_00639810(at<void*>(host.global_00e198c4(), 0x48), unit)) {
        set_white(distance_text, host);
    } else {
        set_colour(distance_text, k0_5f, k0_5f, k0_5f, kOne, host);
    }
    host.gui_text_set_localised_cstring_00abbe50(name_text, name, 1);
    set_white(name_text, host);

    // 0063D785: SETZ CL, upper bytes of ECX left from the child pointer.
    host.widget_vcall_34_set_visible(type_icon, special ? 0 : 1);
    {
        void* instance = host.unit_vcall_140(unit);
        const std::int32_t icon = host.unit_hud_icon_index_0059cb90(instance, nullptr);
        host.widget_vcall_88(type_icon, icon, 0, 1.0f);
    }
    set_white(type_icon, host);

    // 0063D7F7: the health bar width from HP_Icon, height from HP_BG_Icon * scale.
    {
        const float width = host.widget_get_size_00aa6740(hp_icon)[0];  // E+98h
        const float* bg = host.widget_get_size_00aa6740(hp_background);
        const float height = static_cast<float>(static_cast<double>(bg[1]) * scale);  // E+ACh
        const float pair[2] = {width, height};
        host.widget_vcall_58_set_pair(hp_icon, pair);
    }
    host.widget_vcall_50_colour(hp_icon, colour.rgba);

    {
        float anchor[3];  // E+60h
        host.widget_get_resolved_position_00aa6750(name_text, anchor);
        const float half = static_cast<float>(at<float>(name_text, 0x114) / k960);  // E+98h
        anchor[0] = static_cast<float>(static_cast<double>(half) * kHalf + anchor[0]);
        anchor[1] = static_cast<float>(static_cast<double>(anchor[1]) + k0_02);
        host.widget_set_resolved_position_00aa8240(distance_text, anchor);
    }
    float width2;  // E+98h after 0063D8F7
    float right[3];  // E+6Ch
    host.widget_get_resolved_position_00aa6750(name_text, right);
    {
        width2 = static_cast<float>(at<float>(name_text, 0x114) / k960);
        const float bx = static_cast<float>(static_cast<double>(right[0]) + width2 + k0_01);  // E+ACh
        const float by = static_cast<float>(static_cast<double>(right[1]) + k0_03);           // E+B8h
        const float position[3] = {bx, by, kMinus23};
        host.widget_set_resolved_position_00aa8240(hp_background, position);
    }
    host.widget_vcall_4c_set_float(hp_background, k0_4f);
    set_white(hp_background, host);
    {
        const float bx = static_cast<float>(static_cast<double>(right[0]) + width2 + k0_01);
        const float by = static_cast<float>(static_cast<double>(right[1]) + k0_03);
        const float position[3] = {bx, by, kMinus23};
        host.widget_set_resolved_position_00aa8240(hp_icon, position);
    }

    // 0063D9DC: the panel offset around the screen edge by the arrow angle
    // (radians; the angle as passed, before the pi/2 subtraction). Each test
    // follows the image's jump: JBE = <= or unordered, JC = < or unordered,
    // JNC = >= and ordered.
    const double v = arc;
    float slot98 = width2;  // E+98h: overwritten only on the first arm
    bool first_arm;
    if (v > kArcA && kPi >= v) {
        first_arm = true;
    } else if (!(v >= kMinusPi)) {
        first_arm = false;
    } else if (!(kArcD >= v)) {
        first_arm = false;
    } else {
        first_arm = true;
    }
    if (first_arm) {
        // 0063DA0E.
        double r;
        if (!(v > kArcA)) {
            r = v + kPi + kArcH;  // 0063DA20 / DA28: FADDP then FADD
        } else if (!(kPi >= v)) {
            r = v + kPi + kArcH;  // 0063DA24
        } else {
            r = v - kArcA;  // 0063DA1C FSUBRP: ST0(v) - ST1(A)
        }
        slot98 = static_cast<float>(r);
        out_x = static_cast<float>(static_cast<double>(x) + k0_06);
        out_y = static_cast<float>((1.0 - static_cast<double>(slot98) / kArcDiv1) * k0_05 + y);
    } else if (kArcA > v && v >= kArcF) {
        // 0063DA83: FSUBP = v - F.
        const float s = static_cast<float>(v - kArcF);  // E+ACh
        out_x = static_cast<float>((static_cast<double>(x) - static_cast<double>(slot98) * kHalf) +
                                   static_cast<double>(s) * k0_05 / kArcDiv2 - k0_02);
        out_y = static_cast<float>(static_cast<double>(y) + k0_09);
    } else if (kArcF > v && v >= kArcG) {
        // 0063DAEF: FADDP = v + F.
        float margin = kMargin;  // E+ACh
        const float t = static_cast<float>(v + kArcF);  // E+B8h
        if (special && !flagged) {
            margin = kMarginFlag;
        }
        out_x = static_cast<float>(static_cast<double>(x) - slot98 - margin - k0_01);
        out_y = static_cast<float>(static_cast<double>(t) * k0_05 / kArcDiv3 + y);
    } else if (kArcG > v && v >= kArcD) {
        // 0063DB7C: FCHS then FSUBRP = -v - F.
        const float s = static_cast<float>(-v - kArcF);  // E+ACh
        out_x = static_cast<float>((static_cast<double>(x) - static_cast<double>(slot98) * kHalf) +
                                   static_cast<double>(s) * k0_05 / kArcDiv4 - k0_02);
        out_y = static_cast<float>(static_cast<double>(y) - k0_046);
    }
    {
        const float position[3] = {out_x, out_y, flagged ? kOne : 0.0f};
        host.gui_widget_set_local_position_00aa7dc0(panel, position);
    }

    if (special) {
        // 0063DC2B: the command-building icon at the type icon.
        void* holder = widget(record, 0x74);
        host.widget_vcall_34_set_visible(holder, 1);
        float scratch[3];  // E+50h
        host.gui_widget_set_local_position_00aa7dc0(holder,
                                                    host.widget_get_resolved_position_00aa6750(type_icon, scratch));
        host.widget_vcall_34_set_visible(widget(record, 0x70), 0);
        void* icon = find_child(holder, kCommandBuildingIcon, 0x14, host);
        host.widget_vcall_34_set_visible(icon, 1);
        host.widget_vcall_88(icon, at<std::int32_t>(unit, 0x54), 0, 1.0f);
        host.widget_set_resolved_position_00aa8240(icon, host.widget_get_resolved_position_00aa6750(type_icon, scratch));
        host.widget_vcall_34_set_visible(widget(record, 0x80), static_cast<std::int32_t>(flag));
    }

    if (host.entity_vcall_5c_is_kind(unit, 0x1b) && at<std::uint8_t>(unit, 0x720) != 0) {
        set_colour(distance_text, k0_5f, k0_5f, k0_5f, kOne, host);
        set_colour(name_text, k0_5f, k0_5f, k0_5f, kOne, host);
        set_colour(type_icon, k0_5f, k0_5f, k0_5f, kOne, host);
        set_colour(hp_background, k0_5f, k0_5f, k0_5f, kOne, host);
        set_colour(hp_icon, k0_5f, k0_5f, k0_5f, kOne, host);
    }

    // 0063DE7C: the callee releases the by-value distance string.
    release_string(distance, host);
}

// ---------------------------------------------------------------------------
// 00642040 BSP_InGameHudMarkers_BuildUnitMarker
// ---------------------------------------------------------------------------
namespace {

// The party display-name lookup the three arms share: the party table is
// [[00E188A8] + 18CCh + [[00E188A8]+18ECh]*4] + 28h; EDX is unit+9D4h on the
// unit-kind-0Fh arm, else unit->+140h() (the table is read before that call).
const char* party_display_name(void* unit, bool use_9d4, Cc13Pkt642Host& host) {
    void* game = host.global_00e188a8();
    const std::int32_t index = at<std::int32_t>(game, 0x18ec);
    void* entry = at<void*>(game, 0x18cc + static_cast<std::size_t>(index) * 4);
    void* party = at<void*>(entry, 0x28);
    void* key = use_9d4 ? at<void*>(unit, 0x9d4) : host.unit_vcall_140(unit);
    return host.recon_display_name_for_party_00803dc0(party, key);
}

// The "globals.unknown" test: 0041E870 constructs a native string from the
// literal (it writes {0, null} itself first), 00425850 compares the name
// case-insensitively, and the string is released (inline on two arms,
// through 0041DD20 on the off-screen arm).
bool name_is_unknown(const char* name, bool release_through_0041dd20, Cc13Pkt642Host& host) {
    Cc13SweepBNativeString literal{};
    void* built = host.native_string_assign_0041e870(
        &literal, static_cast<const char*>(host.image_global(kGlobalsUnknown)));
    const bool equal = host.native_string_equals_cstring_ci_00425850(built, name);
    if (release_through_0041dd20) {
        host.native_string_destroy_storage_0041dd20(&literal);
    } else {
        release_string(literal, host);
    }
    return equal;
}

// 006425C9 / 006427C2: [00E188D8] must be set and the unit-to-selected
// distance non-zero (00427EB0 on the selected unit first, then on the unit,
// 00427C90 unit - selected, 0042B2F0; FUCOMIP/JNP leaves on equal).
bool selected_distance_nonzero(void* unit, Cc13Pkt642Host& host) {
    void* selected = host.global_00e188d8();
    if (selected == nullptr) {
        return false;
    }
    const float* from = host.entity_world_position_00427eb0(selected);
    float offset[3];  // E+1Ch
    const float* delta = host.vector3_subtract_to_00427c90(host.entity_world_position_00427eb0(unit), offset, from);
    const float length = static_cast<float>(host.vector3_length_0042b2f0(delta));
    return !(length == 0.0f);
}

}  // namespace

// __thiscall(ECX = markers screen), RET 18h; SEH 00C7A3FE (states 0..2 own
// the distance strings). Frame base E = entry ESP - B0h; arguments at E+B4h
// (unit, is_self, is_friendly, is_target, kind_a E+C4h, kind_b E+C8h). The
// image keeps several locals in reused slots (E+10h is the pushed unit, then
// the "no flag" byte over the pointer's upper bytes, then the name pointer;
// E+28h is the saved markers pointer, an out slot of 0063A6C0 on the box arm
// and the arrow x on the off-screen arm); they are separate locals here.
void cc13_pkt642_hud_markers_build_unit_marker_00642040(void* markers, void* unit, std::uint32_t is_self,
                                                        std::uint32_t is_friendly, std::uint32_t is_target,
                                                        std::int32_t kind_a, std::int32_t kind_b,
                                                        Cc13Pkt642Host& host) {
    if (unit == nullptr || at<std::uint8_t>(unit, 0x5c) == 0 || at<std::uint8_t>(unit, 0x5d) != 0 ||
        at<std::uint8_t>(unit, 0x60) != 0 || at<std::uint8_t>(unit, 0x5e) != 0) {
        return;
    }
    {
        void* pushed = unit;  // E+10h
        host.pointer_vector_push_back_004323d0(&at<std::uint8_t>(markers, 0x34), &pushed);
    }
    // 006420B5: only the low byte of E+10h is written; the dword later pushed
    // to 006407A0 keeps the unit pointer's upper three bytes.
    std::uint32_t record_arg = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(unit)) & 0xFFFFFF00u;
    if (!low_byte(is_self) && !low_byte(is_friendly) && !low_byte(is_target)) {
        record_arg |= 1u;
    }
    std::int32_t kind = host.hud_markers_colour_index_00639990(unit);  // EBX, E+30h
    if (at<std::uint8_t>(unit, 0xc8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
    }
    float world[3] = {at<float>(unit, 0xfc), at<float>(unit, 0x100), at<float>(unit, 0x104)};  // E+5Ch
    if (host.entity_vcall_5c_is_kind(unit, 8) && host.entity_is_below_altitude_third_00852860(unit)) {
        void* player = at<void*>(host.global_00e188a8(), 0x1ed4);
        if (at<std::uint8_t>(player, 0xc8) == 0) {
            host.entity_pose_refresh_world_00414db0(player);
        }
        // COMISS; JBE skips on <= or unordered.
        if (at<float>(player, 0x100) > 0.0f) {
            world[1] = 0.0f;
        }
    }

    float screen[3];  // E+88h; with the four box corners it forms float[5][2] at E+68h
    if (host.hud_marker_project_and_clip_00638e50(world, screen)) {
        const bool box = kind == 2 || kind_b == 0 ||
                         (kind_b == 2 &&
                          host.unit_line_of_sight_005220c0(at<void*>(host.global_00e198c4(), 0xcc), unit));
        if (box) {
            // 006421B4: the on-screen box marker.
            void* record = host.hud_markers_acquire_record_006407a0(markers, record_arg);  // EBP, E+1Ch
            std::uint8_t collapsed;  // E+1Bh
            float min_x;             // E+10h
            float min_y;             // E+44h
            float max_x;             // E+34h
            float max_y;             // E+28h
            // 006421E2: the anchor float3 by value at E-24h; the pointer
            // arguments were checked against ESP after each push. The image
            // also stores ESP into E+34h (EH bookkeeping) before the call;
            // 0063A6C0 writes that slot through max_x.
            host.hud_marker_project_unit_bounds_0063a6c0(markers, world[0], world[1], world[2], unit, &collapsed,
                                                         &min_x, &min_y, &max_x, &max_y);
            at<std::uint8_t>(record, 5) = collapsed;
            float corners[10] = {min_x, min_y, min_x, max_y, max_x, max_y, max_x, min_y,
                                 screen[0], screen[1]};  // E+68h..E+8Ch
            const char* name = static_cast<const char*>(host.image_global(kEmptyString));  // E+10h
            if (host.entity_vcall_5c_is_kind(unit, 0x0f)) {
                if ((host.unit_is_squadron_flight_leader_007b8ad0(unit) && low_byte(is_self)) ||
                    low_byte(is_target) || low_byte(is_friendly)) {
                    name = party_display_name(unit, true, host);
                }
            } else {
                name = party_display_name(unit, false, host);
            }
            if (name_is_unknown(name, false, host)) {
                kind = 5;
            }
            void* selected = host.global_00e188d8();
            if (selected != nullptr) {
                if (at<std::uint8_t>(selected, 0xc8) == 0) {
                    host.entity_pose_refresh_world_00414db0(selected);
                }
                if (at<std::uint8_t>(unit, 0xc8) == 0) {
                    host.entity_pose_refresh_world_00414db0(unit);
                }
                // 0064236E: the offset is formed inline as floats, then 0042B2F0.
                const float offset[3] = {
                    static_cast<float>(static_cast<double>(at<float>(unit, 0xfc)) - at<float>(selected, 0xfc)),
                    static_cast<float>(static_cast<double>(at<float>(unit, 0x100)) - at<float>(selected, 0x100)),
                    static_cast<float>(static_cast<double>(at<float>(unit, 0x104)) - at<float>(selected, 0x104))};
                const float length = static_cast<float>(host.vector3_length_0042b2f0(offset));
                if (!(length == 0.0f)) {
                    Cc13SweepBNativeString distance{0, nullptr};  // E+28h, EH state 0
                    host.hud_marker_format_distance_0063e5c0(&distance, unit);
                    host.unit_vcall_140(unit);  // 006423FF: result unused
                    float colour_out[4];        // E+34h
                    const float* colour_ptr = host.unit_health_colour_0077a2f0(colour_out, 1.0f);
                    Cc13Pkt642Colour colour;
                    for (int i = 0; i < 4; ++i) {
                        colour.rgba[i] = colour_ptr[i];  // EBP, E+50h, E+54h, E+58h
                    }
                    // 00642410: same +140h result as the screen's +1Ch unit (unit first).
                    bool same = false;  // BL; EBX was zeroed at 006423D7
                    void* other = at<void*>(markers, 0x1c);
                    if (other != nullptr) {
                        void* mine = host.unit_vcall_140(unit);
                        same = mine == host.unit_vcall_140(other);
                    }
                    const float health = static_cast<float>(host.unit_health_00923be0(host.unit_vcall_140(unit)));
                    Cc13SweepBNativeString copy;  // the by-value argument slot
                    host.native_string_copy_construct_00426060(&copy, &distance);
                    cc13_pkt642_hud_marker_emit_box_marker_0063c620(record, corners, kind, same ? 1u : 0u, 0u, name,
                                                                    unit, copy, health, colour,
                                                                    static_cast<std::uint32_t>(kind_a), host);
                    release_string(distance, host);
                }
            }
        } else {
            // 006424F9: the screen-space unit marker.
            void* record = host.hud_markers_acquire_record_006407a0(markers, record_arg);  // E+34h
            const char* name = static_cast<const char*>(host.image_global(kEmptyString));
            if (host.entity_vcall_5c_is_kind(unit, 0x0f)) {
                if (!low_byte(is_self) || host.unit_is_squadron_flight_leader_007b8ad0(unit)) {
                    name = party_display_name(unit, true, host);
                }
            } else {
                name = party_display_name(unit, false, host);
            }
            if (name_is_unknown(name, false, host)) {
                kind = 5;
            }
            if (selected_distance_nonzero(unit, host)) {
                Cc13SweepBNativeString distance{0, nullptr};  // E+44h, EH state 1
                host.hud_marker_format_distance_0063e5c0(&distance, unit);
                const float health = static_cast<float>(host.unit_health_00923be0(host.unit_vcall_140(unit)));
                host.unit_vcall_140(unit);  // 00642652: result unused
                float colour_out[4];        // E+4Ch
                const float* colour_ptr = host.unit_health_colour_0077a2f0(colour_out, 1.0f);
                Cc13Pkt642Colour colour;
                for (int i = 0; i < 4; ++i) {
                    colour.rgba[i] = colour_ptr[i];
                }
                Cc13SweepBNativeString copy;
                host.native_string_copy_construct_00426060(&copy, &distance);
                cc13_pkt642_hud_marker_write_unit_marker_widgets_0063d1e0(record, unit, screen[0], screen[1],
                                                                          kAnchorAngle, kind, colour, name, copy,
                                                                          health, is_self, host);
                host.native_string_destroy_storage_0041dd20(&distance);
            }
        }
    } else if (low_byte(is_self)) {
        // 00642716: off screen, the arrow at the screen edge (own unit only).
        float x;      // E+28h
        float y;      // E+44h
        float angle;  // E+34h
        cc13_sweepB2_hud_marker_project_offscreen_arrow_00638c60(world, &x, &y, &angle, host);
        const char* name = static_cast<const char*>(host.image_global(kEmptyString));  // EBP
        if (host.entity_vcall_5c_is_kind(unit, 0x0f)) {
            if (host.unit_is_squadron_flight_leader_007b8ad0(unit)) {
                name = party_display_name(unit, true, host);
            }
        } else {
            name = party_display_name(unit, false, host);
        }
        if (name_is_unknown(name, true, host)) {
            kind = 5;
        }
        if (selected_distance_nonzero(unit, host)) {
            void* record = host.hud_markers_acquire_record_006407a0(markers, 0);  // EBX
            Cc13SweepBNativeString distance{0, nullptr};                          // E+10h, EH state 2
            host.hud_marker_format_distance_0063e5c0(&distance, unit);
            const float health = static_cast<float>(host.unit_health_00923be0(host.unit_vcall_140(unit)));
            host.unit_vcall_140(unit);  // 00642856: result unused
            float colour_out[4];        // E+94h
            const float* colour_ptr = host.unit_health_colour_0077a2f0(colour_out, 1.0f);
            Cc13Pkt642Colour colour;
            for (int i = 0; i < 4; ++i) {
                colour.rgba[i] = colour_ptr[i];
            }
            Cc13SweepBNativeString copy;
            host.native_string_copy_construct_00426060(&copy, &distance);
            cc13_pkt642_hud_marker_write_unit_marker_widgets_0063d1e0(record, unit, x, y, angle, kind, colour, name,
                                                                      copy, health, is_self, host);
            host.native_string_destroy_storage_0041dd20(&distance);
        }
    }

    // 00642910: the lead marker for a kind-0Fh unit while +24h is kind 0Fh too.
    if (host.entity_vcall_5c_is_kind(unit, 0x0f)) {
        void* lead = at<void*>(markers, 0x24);
        if (lead != nullptr && host.entity_vcall_5c_is_kind(lead, 0x0f)) {
            cc13_sweepB2_hud_markers_update_lead_marker_00640a20(markers, unit, host);
        }
    }
}

// ---------------------------------------------------------------------------
// 005F65C0 (options commit)
// ---------------------------------------------------------------------------
namespace {

constexpr std::uint32_t kSettings = 0x00F88980u;        // global settings object
constexpr std::uint32_t kSettingsNames = 0x00F88A24u;   // settings +A4h, a native-string vector
constexpr std::uint32_t kXenonManager = 0x00F8ABE8u;    // dword: the Xenon system manager
constexpr std::uint32_t kMetrics = 0x00E1AED4u;         // dword: the metrics sender
constexpr std::uint32_t kEmptyBuffer = 0x00E19654u;     // .bss char buffer used as ""
constexpr std::uint32_t kCallbackVideo = 0x005F0090u;   // 008D6C00 callback when the video mode changed
constexpr std::uint32_t kCallbackNone = 0x00BD53C0u;    // 008D6C00 callback otherwise

const char* literal(std::uint32_t address, Cc13Pkt642Host& host) {
    return static_cast<const char*>(host.image_global(address));
}

// One metrics row: the value string is built into the higher by-value slot
// first, then the key, then 005EFFA0(row, key, value) consumes both,
// 005F6530 appends the row and 005EFDC0 destroys it.
void push_row(void* vector, void* row, std::uint32_t key_literal, Cc13SweepBNativeString value,
              Cc13Pkt642Host& host) {
    Cc13SweepBNativeString key;
    host.native_string_assign_0041e870(&key, literal(key_literal, host));
    const void* built = host.option_row_construct_005effa0(row, key, value);
    host.option_row_vector_push_back_005f6530(vector, built);
    host.option_row_destruct_005efdc0(row);
}

Cc13SweepBNativeString assigned(const char* text, Cc13Pkt642Host& host) {
    Cc13SweepBNativeString value;
    host.native_string_assign_0041e870(&value, text);
    return value;
}

}  // namespace

// __thiscall(ECX = options screen), RET; SEH 00C763A6, frame 4CF8h through
// __alloca_probe (base E = entry ESP - 4D14h). The screen keeps a settings
// copy at +D8h (+150h = copy+78h, +188h = copy+B0h, matching 00F889F8 and
// 00F88A30 in 00F88980). Locals: the row vector E+38h {proxy, first, last,
// end}, rows at E+20h and E+58h (10h bytes each), "none"/player id E+30h,
// the sprintf buffer E+68h, the hardware-info object E+CCh (its +4h and +14h
// vectors are freed inline at the end; +34h, +A34h and +B34h are char
// buffers the CPU, VideoCard and VGADriverVersion rows read).
void cc13_pkt642_options_screen_commit_005f65c0(void* screen, Cc13Pkt642Host& host) {
    void* settings = host.image_global(kSettings);
    void* copy = &at<std::uint8_t>(screen, 0xd8);
    const bool video_changed = host.settings_video_mode_differs_008d4210(copy, settings);  // E+1Fh
    const bool mode_changed = at<std::int32_t>(settings, 0x78) != at<std::int32_t>(screen, 0x150);  // BL
    const std::uint32_t xbox = at<std::uint8_t>(screen, 0x188);
    if (at<std::uint8_t>(settings, 0xb0) != xbox) {
        host.settings_set_xbox_compatibility_008d44c0(settings, xbox);
    }
    host.settings_copy_from_screen_005f52c0(settings, copy);
    host.input_configuration_load_00698a10(&at<std::uint8_t>(host.global_00e188a8(), 0x3c));

    // 005F663C: the metrics rows go out once ([00F88A15] clear) or whenever
    // settings+78h moved.
    if (at<std::uint8_t>(settings, 0x95) == 0 || mode_changed) {
        Cc13SweepBVectorByValue rows;  // E+38h; only first/last/end are written
        rows.first = nullptr;
        rows.last = nullptr;
        rows.end = nullptr;
        alignas(4) std::uint8_t hardware[0x4c3c];  // E+CCh..E+4D08h
        const std::int32_t language = host.system_options_read_language_00992830();  // EDI
        host.hardware_info_construct_009927d0(hardware, language);
        alignas(4) std::uint8_t row[0x10];  // E+20h

        push_row(&rows, row, 0x00CF3AECu /* "GameId" */, assigned(literal(0x00CF3AF4u /* "bp" */, host), host),
                 host);

        Cc13SweepBNativeString player = assigned(literal(0x00CE43ECu /* "none" */, host), host);  // E+30h
        void* xenon = *static_cast<void**>(host.image_global(kXenonManager));
        if (host.xenon_has_selected_user_00a3e510(xenon)) {
            char buffer[0x64];  // E+68h
            buffer[0] = 0;
            const std::uint64_t xuid = host.xenon_active_xuid_00a3eb00(*static_cast<void**>(host.image_global(kXenonManager)));
            host.crt_sprintf_u64_00bf7a6a(buffer, literal(0x00CF3AE4u /* "%llx" */, host), xuid);
            Cc13SweepBNativeString text;  // E+14h
            const Cc13SweepBNativeString* source =
                static_cast<const Cc13SweepBNativeString*>(host.native_string_assign_0041e870(&text, buffer));
            // 005F673E: the inlined assignment (self-test, resize, copy of
            // the resized length when the source is non-empty).
            if (&player != source) {
                host.native_string_resize_0041dd40(&player, source->length, 1);
                if (source->length != 0) {
                    host.crt_memcpy_00bf7680(player.data, source->data, player.length);
                }
            }
            release_string(text, host);
        }
        {
            // 005F679D: the by-value copy of the player id, built in place.
            Cc13SweepBNativeString value;
            value.length = 0;
            value.data = nullptr;
            if (&value != &player) {
                host.native_string_resize_0041dd40(&value, player.length, 1);
                if (player.length != 0) {
                    host.crt_memcpy_00bf7680(value.data, player.data, value.length);
                }
            }
            push_row(&rows, row, 0x00CF3AD8u /* "PlayerID" */, value, host);
        }
        {
            Cc13SweepBNativeString value;
            host.native_string_from_int_004260b0(&value, language);
            push_row(&rows, row, 0x00CF3AD0u /* "LandID" */, value, host);
        }
        push_row(&rows, row, 0x00CF3ACCu /* "D3D" */,
                 assigned(host.hardware_info_gpu_model_name_0098c820(hardware), host), host);
        {
            const std::int32_t id = host.hardware_info_gpu_device_id_0098c870(hardware);
            Cc13SweepBNativeString value;
            host.native_string_from_int_004260b0(&value, id);
            push_row(&rows, row, 0x00CF3AC4u /* "D3DID" */, value, host);
        }
        push_row(&rows, row, 0x00CF3AB8u /* "OSVerzion" */,
                 assigned(host.hardware_info_os_version_0098c810(hardware), host), host);
        push_row(&rows, row, 0x00CEA3FCu /* "Sound" */,
                 assigned(host.hardware_info_sound_device_name_0098d3d0(hardware), host), host);
        push_row(&rows, row, 0x00CF3AB4u /* "CPU" */,
                 assigned(reinterpret_cast<const char*>(hardware + 0x34), host), host);
        {
            const std::uint32_t speed = host.hardware_info_cpu_speed_0098c7f0(hardware);
            Cc13SweepBNativeString value;
            host.native_string_from_unsigned_005f1750(&value, speed);
            push_row(&rows, row, 0x00CF3AA8u /* "CPUSpeed" */, value, host);
        }
        {
            const std::uint32_t memory = host.hardware_info_memory_size_0098c800(hardware);
            Cc13SweepBNativeString value;
            host.native_string_from_unsigned_005f1750(&value, memory);
            push_row(&rows, row, 0x00CE3A94u /* "Memory" */, value, host);
        }
        push_row(&rows, row, 0x00CF3A9Cu /* "VideoCard" */,
                 assigned(reinterpret_cast<const char*>(hardware + 0xa34), host), host);
        push_row(&rows, row, 0x00CF3A88u /* "VGADriverVersion" */,
                 assigned(reinterpret_cast<const char*>(hardware + 0xb34), host), host);

        // 005F6C26: "<width> x <height>" from 00F88994 / 00F88998.
        Cc13SweepBNativeString separator;  // E+14h
        host.native_string_assign_0041e870(&separator, literal(0x00CF3A84u /* " x " */, host));
        Cc13SweepBNativeString width;  // E+20h
        void* width_ptr = host.native_string_signed_decimal_005f1840(
            &width, *static_cast<std::int32_t*>(host.image_global(0x00F88994u)));
        Cc13SweepBNativeString joined;  // E+50h
        void* joined_ptr = host.native_string_concat_004261a0(width_ptr, &joined, &separator);
        Cc13SweepBNativeString resolution;  // E+48h
        host.native_string_concat_int_004263b0(joined_ptr, &resolution,
                                               *static_cast<std::int32_t*>(host.image_global(0x00F88998u)));
        release_string(joined, host);
        release_string(width, host);
        release_string(separator, host);
        {
            const char* text = resolution.data != nullptr ? resolution.data : literal(kEmptyBuffer, host);
            alignas(4) std::uint8_t resolution_row[0x10];  // E+58h
            push_row(&rows, resolution_row, 0x00CF3A78u /* "Resolution" */, assigned(text, host), host);
        }
        void* metrics = *static_cast<void**>(host.image_global(kMetrics));
        if (metrics != nullptr) {
            host.metrics_send_00753570(metrics, &rows);
        }
        at<std::uint8_t>(settings, 0x95) = 1;  // 00F88A15
        release_string(resolution, host);
        release_string(player, host);
        // 005F6DC9: the hardware-info vectors at +14h and +4h, freed inline.
        void* block = at<void*>(hardware, 0x14);
        if (block != nullptr) {
            host.crt_operator_delete_00bf65ac(block);
        }
        block = at<void*>(hardware, 0x4);
        at<void*>(hardware, 0x14) = nullptr;
        at<void*>(hardware, 0x18) = nullptr;
        at<void*>(hardware, 0x1c) = nullptr;
        if (block != nullptr) {
            host.crt_operator_delete_00bf65ac(block);
        }
        at<void*>(hardware, 0x4) = nullptr;
        at<void*>(hardware, 0x8) = nullptr;
        at<void*>(hardware, 0xc) = nullptr;
        host.option_row_vector_destroy_005f5f80(&rows);
    }

    // 005F6E33: the 1 is 00A4C2D0's argument, pushed before 004C1710 runs.
    host.media_manager_ret4_00a4c2d0(host.media_manager_get_or_create_004c1710(), 1);
    host.settings_save_to_profile_008d6c00(settings, video_changed ? kCallbackVideo : kCallbackNone);
    if (at<std::uint8_t>(screen, 0x252) != 0) {
        host.native_string_vector_resize_00427110(host.image_global(kSettingsNames), 0);
        host.gui_manager_reset_screens_00aa0f70(host.gui_manager_get_004c12b0());
        host.object_vcall_60(at<void*>(screen, 0x88), 0);
        at<std::uint8_t>(at<void*>(screen, 0x88), 0x77) = 1;
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x88), 0);
        host.game_request_return_to_title_004d7f90(host.global_00e188a8());
        at<std::uint8_t>(screen, 0x252) = 0;
    } else {
        host.settings_apply_all_008d5b50(settings);
        host.options_screen_build_main_list_005f54f0(screen);
    }
}

// ---------------------------------------------------------------------------
// 00640D70 BSP_InGameHudMarkers_UpdateTargetSectionCallout
// ---------------------------------------------------------------------------
namespace {

const float kFloatMax = f32_bits(0x7F7FFFFFu);     // 00D7A248
const float kFloatLowest = f32_bits(0xFF7FFFFFu);  // 00D7A244
const float kPickRadius = f32_bits(0x3D23D70Au);   // 00CE3910  0.04f

// The eight box corners the image stores at E+90h..E+ECh from the part's
// endpoints a ([part+20h]) and b ([part+24h]).
void box_corners(const float* a, const float* b, float* out) {
    const float points[8][3] = {{a[0], a[1], a[2]}, {a[0], a[1], b[2]}, {a[0], b[1], a[2]}, {a[0], b[1], b[2]},
                                {b[0], a[1], a[2]}, {b[0], a[1], b[2]}, {b[0], b[1], a[2]}, {b[0], b[1], b[2]}};
    std::memcpy(out, points, sizeof points);
}

// One transformed and projected point: 00414DB0 when unit+C8h is clear,
// 004142E0(point, out, unit+CCh), 00638E50(out, screen).
bool project_point(void* unit, const float* point, float* world_out, float* screen, Cc13Pkt642Host& host) {
    if (at<std::uint8_t>(unit, 0xc8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
    }
    host.vector3_transform_affine_point_004142e0(point, world_out, &at<float>(unit, 0xcc));
    return host.hud_marker_project_and_clip_00638e50(world_out, screen);
}

}  // namespace

// __thiscall(ECX = markers screen), RET; EBP frame aligned to 8, SEH
// 00C7A37B (state 0 owns the part-pointer vector at E+50h). Base E = the
// aligned ESP after the four register pushes. Only the first seven of the
// eight corners feed the screen bounds in both bound loops (EBP/EDI = 7),
// as in the image; the nearest-part pass also projects the box centre.
void cc13_pkt642_hud_markers_update_target_section_callout_00640d70(void* markers, Cc13Pkt642Host& host) {
    void* unit = at<void*>(markers, 0x20);  // EBX, E+28h
    if (unit == nullptr) {
        return;
    }
    if (!host.entity_vcall_5c_is_kind(at<void*>(markers, 0x20), 6)) {
        return;
    }
    if (at<std::uint8_t>(host.unit_parts_object_0080e490(at<void*>(markers, 0x20)), 0x14) == 1) {
        return;
    }
    unit = at<void*>(markers, 0x20);
    void* camera = at<void*>(host.global_00e188a8(), 0x19fc);
    float best = kFloatMax;   // E+30h
    void* chosen = nullptr;   // E+14h
    if ((at<std::uint8_t>(camera, 0x5c) & 2) == 0) {
        host.transform_refresh_world_00b6db70(camera);
    }
    const float eye[3] = {at<float>(camera, 0x120), at<float>(camera, 0x124), at<float>(camera, 0x128)};  // E+78h
    Cc13SweepBVectorByValue parts;  // E+50h; the proxy dword is never written
    parts.first = nullptr;
    parts.last = nullptr;
    parts.end = nullptr;

    void* vehicle = host.game_controlled_vehicle_004b4b00();
    void* game = host.global_00e188a8();
    const std::int32_t index = at<std::int32_t>(game, 0x18ec);
    if (vehicle == nullptr || unit == nullptr ||
        at<std::int32_t>(unit, 0x54) == at<std::int32_t>(at<void*>(game, 0x18cc + static_cast<std::size_t>(index) * 4), 0x28)) {
        host.part_pointer_vector_tidy_0063bed0(&parts);
        return;
    }
    {
        // PUSH 5 precedes 00427EB0 (no stack arguments): it is 00804620's.
        const float* position = host.entity_world_position_00427eb0(unit);
        if (!host.unit_sensors_detect_point_00804620(host.game_controlled_vehicle_004b4b00(), position, 5)) {
            host.part_pointer_vector_tidy_0063bed0(&parts);
            return;
        }
    }

    float corners[8][3];  // E+90h
    float world[3];       // E+184h
    float screen[3];      // E+68h
    {
        std::uint32_t iterator[2];  // E+60h {container, element}
        std::uint32_t begin_out[2];  // E+140h
        const std::uint32_t* begun = static_cast<const std::uint32_t*>(host.section_vector_begin_0063ac70(
            &at<std::uint8_t>(host.entity_visual_class_00476b90(unit), 0x3c), begin_out));
        iterator[0] = begun[0];
        iterator[1] = begun[1];
        for (;;) {
            // 00640EB8: the checked iterator against end().
            std::uint8_t* visual = static_cast<std::uint8_t*>(host.entity_visual_class_00476b90(unit));
            const std::uint32_t end = at<std::uint32_t>(visual, 0x44);
            void* container = visual + 0x3c;
            if (at<std::uint32_t>(container, 4) > end) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (iterator[0] == 0 || iterator[0] != static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(container))) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (iterator[1] == end) {
                break;
            }
            void* element = *static_cast<void**>(host.section_iterator_deref_0063ace0(iterator));
            if (element != nullptr) {
                void* sections = static_cast<std::uint8_t*>(element) + 8;  // ESI, E+8Ch
                for (std::uint32_t j = 0;; ++j) {  // E+2Ch
                    const std::uint32_t first = at<std::uint32_t>(sections, 4);
                    if (first == 0) {
                        break;
                    }
                    const std::int32_t count = static_cast<std::int32_t>(at<std::uint32_t>(sections, 8) - first) / 0x2c;
                    if (!(j < static_cast<std::uint32_t>(count))) {
                        break;
                    }
                    void* part = host.part_vector_at_00637660(sections, j);  // EDI, E+4Ch
                    const std::int32_t type = at<std::int32_t>(part, 4);
                    if (type != 8 && type != 5 && type != 6) {
                        continue;
                    }
                    void* slot = part;
                    host.part_pointer_vector_push_back_006403f0(&parts, &slot);
                    const float* pa = at<float*>(part, 0x20);
                    const float* pb = at<float*>(part, 0x24);
                    const float a[3] = {pa[0], pa[1], pa[2]};  // E+40h
                    const float b[3] = {pb[0], pb[1], pb[2]};  // E+34h
                    box_corners(a, b, &corners[0][0]);
                    float min_x = kFloatMax;     // E+18h
                    float min_y = kFloatMax;     // E+1Ch
                    float max_x = kFloatLowest;  // E+24h
                    float max_y = kFloatLowest;  // E+20h
                    for (int k = 0; k < 7; ++k) {
                        project_point(unit, corners[k], world, screen, host);  // AL not tested
                        const float sx = screen[0];
                        const float sy = screen[1];
                        if (!(max_x > sx)) max_x = sx;  // FCOMIP; JA keeps
                        if (!(max_y > sy)) max_y = sy;
                        if (!(sx > min_x)) min_x = sx;
                        if (!(sy > min_y)) min_y = sy;
                    }
                    // 0064114D: COMISS/JBE, the screen centre inside the bounds.
                    if (max_y > k0_5f && max_x > k0_5f && k0_5f > min_y && k0_5f > min_x) {
                        const float sx = static_cast<float>(static_cast<double>(b[0]) + a[0]);  // E+88h
                        const float sy = static_cast<float>(static_cast<double>(b[1]) + a[1]);  // E+84h
                        const float sz = static_cast<float>(static_cast<double>(b[2]) + a[2]);  // E+4Ch
                        float offset[3];
                        offset[0] = static_cast<float>(eye[0] - static_cast<float>(sx * kHalf));
                        offset[1] = static_cast<float>(eye[1] - static_cast<float>(sy * kHalf));
                        offset[2] = static_cast<float>(eye[2] - static_cast<float>(kHalf * sz));
                        const double previous = best;  // FSTP double E+140h
                        const float length = host.vector3_length_00419440(offset);
                        if (previous > length) {  // FCOMIP; JBE = <= or unordered
                            float again[3];  // E+104h, the same values recomputed
                            again[0] = static_cast<float>(eye[0] - static_cast<float>(sx * kHalf));
                            again[1] = static_cast<float>(eye[1] - static_cast<float>(sy * kHalf));
                            again[2] = static_cast<float>(eye[2] - static_cast<float>(kHalf * sz));
                            best = host.vector3_length_00419440(again);
                            chosen = part;
                        }
                    }
                }
            }
            // 00641336: ++iterator, checked.
            if (iterator[0] == 0) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (!(iterator[1] < at<std::uint32_t>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(iterator[0])), 8))) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            iterator[1] += 8;
        }
    }

    if (chosen == nullptr) {
        // 00641357: no part under the centre; the projected part point (box
        // corners and centre) nearest the screen centre within 0.04.
        best = kFloatMax;
        for (std::uint32_t i = 0;; ++i) {
            const std::uint8_t* first = static_cast<const std::uint8_t*>(parts.first);
            if (first == nullptr ||
                !(i < static_cast<std::uint32_t>(static_cast<std::int32_t>(
                          static_cast<const std::uint8_t*>(parts.last) - first) >> 2))) {
                break;
            }
            void* part = *host.part_pointer_vector_at_00637850(&parts, i);
            const float* pa = at<float*>(part, 0x20);
            const float* pb = at<float*>(part, 0x24);
            const float a[3] = {pa[0], pa[1], pa[2]};
            const float b[3] = {pb[0], pb[1], pb[2]};
            float points[9][3];
            box_corners(a, b, &points[0][0]);
            // 006413D2: FMUL forms from the bytes (DC C9 / D8 C9 / DE C9).
            points[8][0] = static_cast<float>((static_cast<double>(b[0]) + a[0]) * kHalf);  // E+F0h
            points[8][1] = static_cast<float>((static_cast<double>(b[1]) + a[1]) * kHalf);
            points[8][2] = static_cast<float>(kHalf * (static_cast<double>(b[2]) + a[2]));
            for (int k = 0; k < 9; ++k) {
                float projected[3];  // E+158h
                if (!project_point(unit, points[k], world, projected, host)) {
                    continue;
                }
                projected[2] = 0.0f;  // E+160h (XORPS store)
                float delta[3];       // E+104h
                // FLD ST0; DE E2 FSUBRP ST2: 0.5 - x; then 0.5 - y; FLDZ, DC E0: 0 - 0.
                delta[0] = static_cast<float>(kHalf - projected[0]);
                delta[1] = static_cast<float>(kHalf - projected[1]);
                delta[2] = 0.0f;
                const float distance = host.vector3_length_00419440(delta);  // E+2Ch
                if (kPickRadius > distance && best > distance) {  // both FCOMIP; JBE
                    best = distance;
                    chosen = *host.part_pointer_vector_at_00637850(&parts, i);
                }
            }
        }
        if (chosen == nullptr) {
            // 006418C2: the vector's storage freed inline (not zeroed).
            if (parts.first != nullptr) {
                host.crt_operator_delete_00bf65ac(parts.first);
            }
            return;
        }
    }

    // 006415F6: the localisation key by part type.
    std::uint32_t key;
    const std::int32_t type = at<std::int32_t>(chosen, 4);
    if (type == 8) {
        key = 0x00CECE48u;  // "ingame.sections_magazine"
    } else if (type == 5) {
        key = 0x00CECE24u;  // "ingame.sections_engine"
    } else if (type == 6) {
        key = 0x00CECE0Cu;  // "ingame.sections_fuel"
    } else {
        host.part_pointer_vector_tidy_0063bed0(&parts);
        return;
    }
    float a[3];  // E+40h
    float b[3];  // E+34h
    host.hud_marker_part_endpoints_00638b20(chosen, a, b);
    box_corners(a, b, &corners[0][0]);
    // E+164h..E+18Fh: the four corners, then the transform output slot,
    // whose x/y the callee reads as corners[4] (the last transformed point).
    float block[11];
    float min_x = kFloatMax;     // E+18h
    float min_y = kFloatMax;     // E+1Ch
    float max_x = kFloatLowest;  // E+20h
    float max_y = kFloatLowest;  // E+24h
    for (int k = 0; k < 7; ++k) {
        project_point(unit, corners[k], &block[8], screen, host);
        const float sx = screen[0];
        const float sy = screen[1];
        if (!(max_x > sx)) max_x = sx;
        if (!(max_y > sy)) max_y = sy;
        if (!(sx > min_x)) min_x = sx;
        if (!(sy > min_y)) min_y = sy;
    }
    block[0] = min_x;
    block[1] = min_y;
    block[2] = min_x;
    block[3] = max_y;
    block[4] = max_x;
    block[5] = max_y;
    block[6] = max_x;
    block[7] = min_y;
    void* record = host.hud_markers_acquire_record_006407a0(markers, 0);
    // 00641865: the colour argument is the 16 raw bytes of the part-pointer
    // vector object at E+50h (proxy, first, last, end), copied as they are.
    Cc13Pkt642Colour colour;
    static_assert(sizeof(Cc13SweepBVectorByValue) == sizeof(Cc13Pkt642Colour) || sizeof(void*) != 4);
    std::memcpy(&colour, &parts, sizeof colour);
    Cc13SweepBNativeString empty;
    host.native_string_assign_0041e870(&empty, literal(kEmptyString, host));
    cc13_pkt642_hud_marker_emit_box_marker_0063c620(record, block, 3, 0, 1, literal(key, host), nullptr, empty, 0.0f,
                                                    colour, 0, host);
    if (parts.first != nullptr) {
        host.crt_operator_delete_00bf65ac(parts.first);
    }
}

// ---------------------------------------------------------------------------
// 005F0CD0 BSP_OptionsScreen_RefreshHelpLine
// ---------------------------------------------------------------------------
namespace {

constexpr std::uint32_t kCommandBar = 0x00E1930Cu;  // dword: the main-menu command bar
constexpr std::uint32_t kHelpMode = 0x00E19650u;    // dword: the options help-line mode

// The inlined literal build of this function: {0, null}, 0041DD40(&s,
// length, 1), then 00BF7680(data, literal, length + 1) when data is set.
Cc13SweepBNativeString help_literal(std::uint32_t address, std::uint32_t length, Cc13Pkt642Host& host) {
    Cc13SweepBNativeString text;
    build_literal(text, address, length, host);
    return text;
}

Cc13SweepBNativeString help_assigned(std::uint32_t address, Cc13Pkt642Host& host) {
    Cc13SweepBNativeString text{};
    host.native_string_assign_0041e870(&text, literal(address, host));
    return text;
}

void rebuild_bar(const Cc13SweepB2Host::CommandTriple (&triples)[5], Cc13Pkt642Host& host) {
    host.command_bar_rebuild_0054b530(*static_cast<void**>(host.image_global(kCommandBar)), triples);
}

constexpr std::uint32_t kCmdA2 = 0xFFFFFFA2u;  // PUSH -5Eh
constexpr std::uint32_t kCmdA7 = 0xFFFFFFA7u;  // PUSH -59h
constexpr std::uint32_t kCmdB3 = 0xFFFFFFB3u;  // PUSH -4Dh
constexpr std::uint32_t kCmdAF = 0xFFFFFFAFu;  // PUSH -51h
constexpr std::uint32_t kCmdA3 = 0xFFFFFFA3u;  // PUSH -5Dh

}  // namespace

// __fastcall(ECX = options screen, not read), RET; EBP frame aligned to 8,
// SEH 00C75BB9 (states for each temporary string). Three rebuilds of the
// command bar [00E1930C] (0054B530, five {command dword, NativeString*,
// placement} triples, 15 pushed dwords): blank labels, the default
// cancel/change/navigate/default/accept set, then the set for the mode
// [00E19650] (0, 4 or 7; any other mode stops after the second).
void cc13_pkt642_options_screen_refresh_help_line_005f0cd0(void* screen, Cc13Pkt642Host& host) {
    (void)screen;
    {
        // 005F0CF4: five " " labels (00CE3A90) built at E+30h, 24h, 1Ch, 14h, 0Ch.
        Cc13SweepBNativeString s30 = help_literal(0x00CE3A90u, 1, host);
        Cc13SweepBNativeString s24 = help_literal(0x00CE3A90u, 1, host);
        Cc13SweepBNativeString s1c = help_literal(0x00CE3A90u, 1, host);
        Cc13SweepBNativeString s14 = help_literal(0x00CE3A90u, 1, host);
        Cc13SweepBNativeString s0c = help_literal(0x00CE3A90u, 1, host);
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {0, &s0c, 1}, {0, &s14, 1}, {0, &s1c, 1}, {0, &s24, 1}, {0, &s30, 1}};
        rebuild_bar(triples, host);
        release_string(s0c, host);
        release_string(s14, host);
        release_string(s1c, host);
        release_string(s24, host);
        release_string(s30, host);
    }
    // 005F0F14: E+2Fh = the 00F88A30 byte, read before the second build.
    const std::uint8_t layout = *static_cast<std::uint8_t*>(host.image_global(0x00F88A30u));
    std::uint32_t slot_0c;  // the dword at E+0Ch, reused by mode 4 below
    {
        Cc13SweepBNativeString s0c = help_literal(0x00CEBE80u, 0x0e, host);  // globals.cancel
        Cc13SweepBNativeString s14 = help_literal(0x00CEB410u, 0x0e, host);  // globals.change
        Cc13SweepBNativeString s1c = help_literal(0x00CEB394u, 0x10, host);  // globals.navigate
        Cc13SweepBNativeString s24 = help_literal(0x00CF392Cu, 0x0f, host);  // globals.default
        Cc13SweepBNativeString s30 = help_literal(0x00CED4D4u, 0x0e, host);  // globals.accept
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {kCmdA2, &s30, 0}, {kCmdA7, &s24, 1}, {kCmdB3, &s1c, 1}, {kCmdAF, &s14, 1}, {kCmdA3, &s0c, 2}};
        rebuild_bar(triples, host);
        release_string(s30, host);
        release_string(s24, host);
        release_string(s1c, host);
        release_string(s14, host);
        release_string(s0c, host);
        slot_0c = s0c.length;
    }

    const std::uint32_t mode = *static_cast<std::uint32_t*>(host.image_global(kHelpMode));
    if (mode == 7) {
        // 005F1160.
        Cc13SweepBNativeString s38 = help_assigned(0x00CEBE80u, host);  // globals.cancel
        Cc13SweepBNativeString s80 = help_assigned(0x00CEB410u, host);  // globals.change
        Cc13SweepBNativeString s50 = help_assigned(0x00CEB394u, host);  // globals.navigate
        Cc13SweepBNativeString s90 = help_assigned(kEmptyString, host);
        Cc13SweepBNativeString s60 = help_assigned(0x00CED4D4u, host);  // globals.accept
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {kCmdA2, &s60, 0}, {0, &s90, 1}, {kCmdB3, &s50, 1}, {kCmdAF, &s80, 1}, {kCmdA3, &s38, 2}};
        rebuild_bar(triples, host);
        release_string(s60, host);
        release_string(s90, host);
        release_string(s50, host);
        release_string(s80, host);
        release_string(s38, host);
    } else if (mode == 4) {
        // 005F12DA: only the low byte of the E+0Ch dword is rewritten (0A5h
        // with the layout byte set, else 0AFh); the pushed dword keeps the
        // upper bytes of the earlier string length stored there.
        const std::uint8_t command = static_cast<std::uint8_t>((layout != 0 ? 0xf6 : 0) + 0xaf);
        slot_0c = (slot_0c & 0xFFFFFF00u) | command;
        Cc13SweepBNativeString s58 = help_assigned(layout != 0 ? 0x00CF3920u /* FE.layout */ : 0x00CEB410u, host);
        Cc13SweepBNativeString s48 = help_assigned(0x00CEBE80u, host);  // globals.cancel
        Cc13SweepBNativeString s40 = help_assigned(0x00CEB394u, host);  // globals.navigate
        Cc13SweepBNativeString s70 = help_assigned(0x00CF392Cu, host);  // globals.default
        Cc13SweepBNativeString sa0 = help_assigned(0x00CED4D4u, host);  // globals.accept
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {kCmdA2, &sa0, 0}, {kCmdA7, &s70, 1}, {kCmdB3, &s40, 1}, {slot_0c, &s58, 1}, {kCmdA3, &s48, 2}};
        rebuild_bar(triples, host);
        release_string(sa0, host);
        release_string(s70, host);
        release_string(s40, host);
        release_string(s48, host);
        release_string(s58, host);
    } else if (mode == 0) {
        // 005F146A.
        Cc13SweepBNativeString sa8 = help_assigned(0x00CEB3A8u, host);  // globals.back
        Cc13SweepBNativeString s98 = help_assigned(kEmptyString, host);
        Cc13SweepBNativeString s88 = help_assigned(kEmptyString, host);
        Cc13SweepBNativeString s78 = help_assigned(0x00CEB394u, host);  // globals.navigate
        Cc13SweepBNativeString s68 = help_assigned(0x00CEB3ECu, host);  // globals.select
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {kCmdA2, &s68, 0}, {kCmdB3, &s78, 1}, {0, &s88, 1}, {0, &s98, 1}, {kCmdA3, &sa8, 2}};
        rebuild_bar(triples, host);
        release_string(s68, host);
        release_string(s78, host);
        release_string(s88, host);
        release_string(s98, host);
        release_string(sa8, host);
    }
}

}  // namespace bsp
