// cc13_sweepB4: fourth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB4, docs/CC13_SWEEPB4.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB..B3): float expressions follow the image's
// operation order with a float store wherever the image stores to a float
// slot; intermediate x87 values are kept in double (53-bit precision control),
// and double constants are applied in double. EH frames are noted, not
// modelled. Register-register x87 operand directions were read from the bytes
// (local/sweepB4_dis.py decodes the DC/DE forms by the Intel encoding).

#include "bsp/cc13_sweepB4.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>

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

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepBHost& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined debug-checked iterator dereference: container {+0 proxy,
// +4 head}; a null container or node == head calls 00BF6713, which may
// return, and the read goes ahead either way.
void check_deref(void* container, void* node, Cc13SweepB2Host& host) {
    if (container == nullptr) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (node == at<void*>(container, 4)) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 007ADD70 BSP_EntityCommand_PathCursorFollowStep
// __thiscall(ECX = cursor; const float* pos, float radius, int count,
// float heading, float factor, bool force), RET 18h, AL.
// Cursor: +4h path (vtable +0Ch count), +8h index, +0Ch follow mode
// (PATH_FM_SIMPLE 1, PINGPONG 2, CIRCLE 3), +10h direction byte; cursor
// vtable +4h point(out, index), +8h set_index(index).
//
// Head 007ADD70-007ADDF0: the point count is read once (007ADD84). Modes 1/2:
// direction set -> step +1 and limit count - index, else step -1 and limit
// index. Mode 3: limit count - 2, step from the direction byte. Any other
// mode: limit = the count argument and the step slot [ESP+10h] is NEVER
// written (uninitialised stack). Lua only supplies modes 1..3
// (docs/SHIP_AI_PATH_CURSOR.md); this reconstruction uses step 0 there, which
// is a documented divergence, not the binary. When the count argument's LOW
// BYTE is zero the limit is clamped to 1 (007ADDD9/007ADDE8).
//
// Scan 007ADE00-007ADF5B, one point per pass from the start index: dx/dz
// against pos[0]/pos[1] (float stores), d2 = (float)(dx*dx + dz*dz); inside
// when (float)(radius*radius) > d2 (FCOMIP + JBE: NaN is outside). The index
// steps and wraps (>= count -> 0, < 0 -> + count) after the test either way.
// Inside with 0 < factor < 1 (COMISS + JBE both ways): the lookahead
// 007ADEAF-007ADF44 -- a = (float)atan2(dz, dx) (FXCH puts dx in ST0, dz in
// ST1 for _CIatan2), a = (float)(pi/2 (double 00CE3830) - a), a < 0 ->
// a = (float)(a + 2pi (double 00CE3828)); s = (float)sin((float)00438B10(
// heading, a)); r = (float)(((1 - factor) * |s| + factor) * radius); still
// inside only when r*r > d2 (FCOMIP + JA). A pass that stays inside stops
// the scan with advances = pass + 1; otherwise the scan continues.
//
// No inside pass: force false -> false; force true -> one advance (bl set).
// Advance loop 007ADF90-007ADFF0: each pass re-reads the path, stops on a
// null path, a count <= 0, or a PATH_FM_SIMPLE end (direction set and index
// == count - 1, or direction clear and index 0; the count is called again for
// that test); otherwise clears bl, 007ADCC0 -> vtable +8h. Returns false when
// the advances run out. A stop falls to 007ADFFE: bl clear -> false, bl set
// (no advance taken yet) -> the end test 007AE002, which answers true for a
// null path, a count <= 0 or a PATH_FM_SIMPLE end, false otherwise. A limit
// <= 0 goes to the end test directly (007ADDFA).
// No RNG draws.
bool cc13_sweepB4_path_cursor_follow_step_007add70(void* cursor, const float* pos, float radius,
                                                   std::int32_t count, float heading, float factor,
                                                   bool force, Cc13SweepB4Host& host) {
    // The PATH_FM_SIMPLE end test shared by 007ADF90 and 007AE002: true means
    // "stop" (null path, count <= 0, or simple-mode end point).
    auto at_end = [&]() -> bool {
        void* path = at<void*>(cursor, 4);
        if (path == nullptr || host.path_vcall_0c_count(path) <= 0) {
            return true;
        }
        if (at<std::int32_t>(cursor, 0xC) == 1) {
            if (at<std::uint8_t>(cursor, 0x10) != 0) {
                return at<std::int32_t>(cursor, 8) ==
                       host.path_vcall_0c_count(at<void*>(cursor, 4)) - 1;
            }
            return at<std::int32_t>(cursor, 8) == 0;
        }
        return false;
    };

    std::int32_t index = at<std::int32_t>(cursor, 8);
    const std::int32_t point_count = host.path_vcall_0c_count(at<void*>(cursor, 4));
    const std::int32_t mode = at<std::int32_t>(cursor, 0xC);
    const bool forward = at<std::uint8_t>(cursor, 0x10) != 0;
    std::int32_t step = 0;  // uninitialised in the image for modes outside 1..3
    std::int32_t limit;
    if (mode == 1 || mode == 2) {
        step = forward ? 1 : -1;
        limit = forward ? point_count - index : index;
    } else if (mode == 3) {
        limit = point_count - 2;
        step = forward ? 1 : -1;
    } else {
        limit = count;
    }
    const float radius_sq = static_cast<float>(static_cast<double>(radius) * radius);
    if ((count & 0xFF) == 0 && limit > 1) {
        limit = 1;
    }
    if (limit <= 0) {
        return at_end();
    }

    bool hit = false;  // BL
    std::int32_t advances = 0;
    for (std::int32_t pass = 0; pass < limit; ++pass) {
        float scratch[3];
        const float* point = host.path_cursor_vcall_04_point(cursor, scratch, index);
        const float px = point[0];
        const float pz = point[2];
        const float dx = px - pos[0];
        const float dz = pz - pos[1];
        const float dist_sq =
            static_cast<float>(static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
        hit = radius_sq > dist_sq;
        index += step;
        if (index >= point_count) {
            index = 0;
        } else if (index < 0) {
            index += point_count;
        }
        if (!hit) {
            continue;
        }
        if (factor > 0.0f && 1.0f > factor) {
            float a = static_cast<float>(host.crt_atan2_00bf701a(dz, dx));
            a = static_cast<float>(f64_bits(0x3FF921FB60000000ull) - a);  // 00CE3830
            if (0.0f > a) {
                a = static_cast<float>(a + f64_bits(0x401921FB60000000ull));  // 00CE3828
            }
            const float turn = static_cast<float>(host.math_subtract_wrapped_angle_00438b10(heading, a));
            const float s = static_cast<float>(std::sin(static_cast<double>(turn)));
            const float abs_s = std::fabs(s);
            const float reach = static_cast<float>(
                ((1.0 - factor) * abs_s + factor) * static_cast<double>(radius));
            hit = static_cast<double>(reach) * reach > dist_sq;
            if (!hit) {
                continue;
            }
        }
        // 007ADF6B: pass + 1 can only be zero or negative on overflow.
        advances = pass + 1;
        break;
    }

    if (!hit) {
        if (!force) {
            return false;
        }
        advances = 1;
        hit = true;
    }
    for (;;) {
        if (at_end()) {
            // 007ADFFE
            return hit ? at_end() : false;
        }
        hit = false;
        const std::int32_t next = host.path_cursor_next_index_007adcc0(cursor);
        host.path_cursor_vcall_08_set_index(cursor, next);
        if (--advances <= 0) {
            return false;
        }
    }
}

// ---------------------------------------------------------------------------
// HUD counter rows (segment 15). The three layouts share: widget +34h is
// set-visible, +58h/+48h take a float pair, 00AA7DC0 takes &entry+30h, and
// the by-value native-string arguments are released (in argument order) at
// the end. Text widgets +24h/+28h hold their glyph height at +114h.

namespace {

void* widget(void* entry, std::size_t offset) {
    return at<void*>(entry, offset);
}

// The inlined native-string assignment entry+offset = source (0052A16E,
// 0052A1F9, 0052A4B2, 0052A4E9): when the addresses differ, 0041DD40(dst,
// source length, 1), then memcpy of the destination's (new) length when the
// source length is non-zero.
void assign_string(void* entry, std::size_t offset, const Cc13SweepBNativeString& source,
                   Cc13SweepBHost& host) {
    auto* dest = static_cast<std::uint8_t*>(entry) + offset;
    if (static_cast<const void*>(dest) == static_cast<const void*>(&source)) {
        return;
    }
    host.native_string_resize_0041dd40(dest, source.length, 1);
    if (source.length != 0) {
        host.crt_memcpy_00bf7680(at<void*>(dest, 4), source.data, at<std::uint32_t>(dest, 0));
    }
}

// The two-text scale of 0052A100/0052A470: each text's +114h / 960.0 (double
// 00CEC380), float-stored; the +24h value when it is greater (FCOMIP + JBE,
// so a NaN takes the +28h value), then raised to 0.1f unless it is above the
// double 0.1 at 00D7A3A0 (JA).
float two_text_scale(void* entry) {
    const float right = static_cast<float>(at<float>(widget(entry, 0x28), 0x114) / 960.0);
    const float left = static_cast<float>(at<float>(widget(entry, 0x24), 0x114) / 960.0);
    float scale = left > right ? left : right;
    if (!(scale > f64_bits(0x3FB99999A0000000ull))) {
        scale = 0.1f;
    }
    return scale;
}

// (entry+30h - entry+3Ch) / entry+60h, float-stored.
float counter_fraction(void* entry) {
    return static_cast<float>((static_cast<double>(at<float>(entry, 0x30)) - at<float>(entry, 0x3C)) /
                              at<float>(entry, 0x60));
}

}  // namespace

// 0052A100 BSP_HudCounter_LayoutValue
// __fastcall(ECX = entry; native string a, native string b, int v0, int v1),
// RET 18h. EH frame 00C6B800 (state 1 = both string arguments live).
// +9Ch = 1 (stored before the first call); 00AA6820(+0h, 1), +0h->+34h(1),
// 00AA6820(+0h, 0); +18h/+1Ch/+20h hidden. +88h = a; +10h->+88h(v0, 0, 1.0f);
// +24h: +58h({0.5, 0.5}), dword +100h = 0, byte +0FCh = 0, 00ABAED0(&+88h, 1).
// The same for +90h = b, +14h (v1) and +28h. scale = two_text_scale; +30h =
// (float)(+54h + scale); 00AA7DC0 on +8h and +2Ch; f = counter_fraction ->
// +0Ch->+48h({f, 1.0f}). P = 00AA6750(+2Ch), Q = 00AA6750(+0Ch) (the
// returned pointer); 00AA8240(+2Ch, {(float)(Q.x - 0.02 (double 00D7A2F8)),
// P.y, P.z}). h = 00AA6740(+2Ch)[1], then +2Ch->+58h({(float)(00AA6740(+0Ch)
// [0] * f + 0.035 (double 00CED078)), h}). +8h/+4h/+0Ch hidden. a then b
// released.
void cc13_sweepB4_hud_counter_layout_value_0052a100(void* entry, Cc13SweepBNativeString a,
                                                    Cc13SweepBNativeString b, std::int32_t v0,
                                                    std::int32_t v1, Cc13SweepB4Host& host) {
    at<std::int32_t>(entry, 0x9C) = 1;
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 1);
    host.widget_vcall_34_set_visible(widget(entry, 0), 1);
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x18), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x1C), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x20), 0);

    const float half[2] = {0.5f, 0.5f};
    assign_string(entry, 0x88, a, host);
    host.widget_vcall_88(widget(entry, 0x10), v0, 0, 1.0f);
    host.widget_vcall_58_set_pair(widget(entry, 0x24), half);
    at<std::int32_t>(widget(entry, 0x24), 0x100) = 0;
    at<std::uint8_t>(widget(entry, 0x24), 0xFC) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x24), static_cast<std::uint8_t*>(entry) + 0x88, 1);

    assign_string(entry, 0x90, b, host);
    host.widget_vcall_88(widget(entry, 0x14), v1, 0, 1.0f);
    host.widget_vcall_58_set_pair(widget(entry, 0x28), half);
    at<std::int32_t>(widget(entry, 0x28), 0x100) = 0;
    at<std::uint8_t>(widget(entry, 0x28), 0xFC) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x28), static_cast<std::uint8_t*>(entry) + 0x90, 1);

    const float scale = two_text_scale(entry);
    float* position = &at<float>(entry, 0x30);
    *position = static_cast<float>(static_cast<double>(at<float>(entry, 0x54)) + scale);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 8), position);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 0x2C), position);
    const float fraction[2] = {counter_fraction(entry), 1.0f};
    host.widget_vcall_48_set_pair(widget(entry, 0xC), fraction);

    float p[3];
    host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p);
    float q_out[3];
    const float* q = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q_out);
    const float qx = q[0];
    const float moved[3] = {static_cast<float>(qx - f64_bits(0x3F947AE140000000ull)), p[1], p[2]};
    host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved);
    const float height = host.gui_widget_get_size_00aa6740(widget(entry, 0x2C))[1];
    const float width = host.gui_widget_get_size_00aa6740(widget(entry, 0xC))[0];
    const float size[2] = {
        static_cast<float>(static_cast<double>(width) * fraction[0] + f64_bits(0x3FA1EB8520000000ull)), height};
    host.widget_vcall_58_set_pair(widget(entry, 0x2C), size);
    host.widget_vcall_34_set_visible(widget(entry, 8), 0);
    host.widget_vcall_34_set_visible(widget(entry, 4), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0xC), 0);
    release_string(a, host);
    release_string(b, host);
}

// 0052A470 BSP_HudCounter_LayoutTextPair (hypothesis)
// __fastcall(ECX = entry; native string a, native string b), RET 10h.
// EH frame 00C6B820 (state 1 = both string arguments live).
// 00ABBE50(+24h, "" (00CE3A0C), 1) and (+28h, "", 1); +88h = a, +90h = b;
// byte +0FCh = 0 on +24h then +28h; 00ABAED0(+24h, &a, 1) and (+28h, &b, 1)
// -- the ARGUMENTS, not the copies. scale = two_text_scale; t = (float)(+54h
// + scale). P = 00AA6750(+2Ch); f = counter_fraction (current +30h); Q =
// 00AA6750(+0Ch); 00AA8240(+2Ch, {(float)(Q.x - 0.02), P.y, P.z}); h =
// 00AA6740(+2Ch)[1]; +2Ch->+58h({(float)(00AA6740(+0Ch)[0] * f + 0.035), h}).
// Only when t > +30h (FCOMIP + JBE; NaN skips): +30h = t, 00AA7DC0 on +8h and
// +2Ch, f' = counter_fraction -> +0Ch->+48h({f', 1.0f}), P' = 00AA6750(+2Ch),
// the scale recomputed, Q' = 00AA6750(+0Ch), 00AA8240(+2Ch, {(float)(Q'.x -
// 0.019 (double 00CED088)), P'.y, P'.z}), +2Ch->+58h({(float)(scale + 0.073
// (double 00CED080)), 00AA6740(+2Ch)[1]}), and +8h/+4h/+0Ch hidden. a then b
// released.
void cc13_sweepB4_hud_counter_layout_text_pair_0052a470(void* entry, Cc13SweepBNativeString a,
                                                        Cc13SweepBNativeString b, Cc13SweepB4Host& host) {
    const char* empty = static_cast<const char*>(host.image_global(0x00CE3A0C));
    host.gui_text_set_localised_cstring_00abbe50(widget(entry, 0x24), empty, 1);
    host.gui_text_set_localised_cstring_00abbe50(widget(entry, 0x28), empty, 1);
    assign_string(entry, 0x88, a, host);
    assign_string(entry, 0x90, b, host);
    at<std::uint8_t>(widget(entry, 0x24), 0xFC) = 0;
    at<std::uint8_t>(widget(entry, 0x28), 0xFC) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x24), &a, 1);
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x28), &b, 1);

    const float target = static_cast<float>(static_cast<double>(at<float>(entry, 0x54)) + two_text_scale(entry));
    float p[3];
    host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p);
    const float fraction = counter_fraction(entry);
    float q_out[3];
    const float* q = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q_out);
    const float qx = q[0];
    const float moved[3] = {static_cast<float>(qx - f64_bits(0x3F947AE140000000ull)), p[1], p[2]};
    host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved);
    const float height = host.gui_widget_get_size_00aa6740(widget(entry, 0x2C))[1];
    const float width = host.gui_widget_get_size_00aa6740(widget(entry, 0xC))[0];
    const float size[2] = {
        static_cast<float>(static_cast<double>(width) * fraction + f64_bits(0x3FA1EB8520000000ull)), height};
    host.widget_vcall_58_set_pair(widget(entry, 0x2C), size);

    float* position = &at<float>(entry, 0x30);
    if (target > *position) {
        *position = target;
        host.gui_widget_set_local_position_00aa7dc0(widget(entry, 8), position);
        host.gui_widget_set_local_position_00aa7dc0(widget(entry, 0x2C), position);
        const float pair[2] = {counter_fraction(entry), 1.0f};
        host.widget_vcall_48_set_pair(widget(entry, 0xC), pair);
        float p2[3];
        host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p2);
        const float scale = two_text_scale(entry);
        float q2_out[3];
        const float* q2 = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q2_out);
        const float q2x = q2[0];
        const float moved2[3] = {static_cast<float>(q2x - f64_bits(0x3F9374BC60000000ull)), p2[1], p2[2]};
        host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved2);
        const float height2 = host.gui_widget_get_size_00aa6740(widget(entry, 0x2C))[1];
        const float size2[2] = {static_cast<float>(scale + f64_bits(0x3FB2B020C0000000ull)), height2};
        host.widget_vcall_58_set_pair(widget(entry, 0x2C), size2);
        host.widget_vcall_34_set_visible(widget(entry, 8), 0);
        host.widget_vcall_34_set_visible(widget(entry, 4), 0);
        host.widget_vcall_34_set_visible(widget(entry, 0xC), 0);
    }
    release_string(a, host);
    release_string(b, host);
}

// 0052A860 BSP_HudCounter_LayoutCountdown
// __fastcall(ECX = entry; native string text, float seconds), RET 0Ch.
// EH frame 00C6B838 (state 0 = the string argument live).
// 00AA6820(+0h, 1), +0h->+34h(1), 00AA6820(+0h, 0); +9Ch = 2; +18h/+1Ch/+20h/
// +10h/+14h hidden. +24h: +58h({0.5, 0.5}), dword +100h = 0, 00ABAED0(&text,
// 1). scale = (float)(+24h.+114h / 960.0), raised to 0.04f (00CE3910) unless
// above it (FCOMIP + JA). +28h: dword +100h = 0, +58h({scale,
// 00AA6740(+28h)[1]}). +30h = (float)((+54h + scale) + (+54h - +3Ch)) (the
// bytes: FADDP ST2 then FADDP ST1); 00AA7DC0 on +8h and +2Ch; +2Ch shown;
// +0Ch->+48h({counter_fraction, 1.0f}); P = 00AA6750(+2Ch), Q = 00AA6750(+0Ch);
// 00AA8240(+2Ch, {(float)(Q.x + 0.013 (double 00CED090)), P.y, P.z});
// +2Ch->+58h({(float)(scale + 0.035), 00AA6740(+2Ch)[1]}); +8h/+4h/+0Ch
// hidden. +98h = seconds; +0A0h = 0 when seconds >= 3600.0 (FCOMI + JB, so
// NaN falls through), else 1 when seconds >= 60.0, else 2. text released.
void cc13_sweepB4_hud_counter_layout_countdown_0052a860(void* entry, Cc13SweepBNativeString text,
                                                        float seconds, Cc13SweepB4Host& host) {
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 1);
    host.widget_vcall_34_set_visible(widget(entry, 0), 1);
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 0);
    at<std::int32_t>(entry, 0x9C) = 2;
    host.widget_vcall_34_set_visible(widget(entry, 0x18), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x1C), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x20), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x10), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x14), 0);

    const float half[2] = {0.5f, 0.5f};
    host.widget_vcall_58_set_pair(widget(entry, 0x24), half);
    at<std::int32_t>(widget(entry, 0x24), 0x100) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x24), &text, 1);
    float scale = static_cast<float>(at<float>(widget(entry, 0x24), 0x114) / 960.0);
    if (!(scale > 0.04f)) {
        scale = 0.04f;
    }
    at<std::int32_t>(widget(entry, 0x28), 0x100) = 0;
    const float* size = host.gui_widget_get_size_00aa6740(widget(entry, 0x28));
    const float scaled[2] = {scale, size[1]};
    host.widget_vcall_58_set_pair(widget(entry, 0x28), scaled);

    const double base = at<float>(entry, 0x54);
    float* position = &at<float>(entry, 0x30);
    *position = static_cast<float>((base + scale) + (base - at<float>(entry, 0x3C)));
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 8), position);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 0x2C), position);
    host.widget_vcall_34_set_visible(widget(entry, 0x2C), 1);
    const float fraction[2] = {counter_fraction(entry), 1.0f};
    host.widget_vcall_48_set_pair(widget(entry, 0xC), fraction);

    float p[3];
    host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p);
    float q_out[3];
    const float* q = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q_out);
    const float qx = q[0];
    const float moved[3] = {static_cast<float>(qx + f64_bits(0x3F8A9FBE80000000ull)), p[1], p[2]};
    host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved);
    const float* size2 = host.gui_widget_get_size_00aa6740(widget(entry, 0x2C));
    const float scaled2[2] = {static_cast<float>(scale + f64_bits(0x3FA1EB8520000000ull)), size2[1]};
    host.widget_vcall_58_set_pair(widget(entry, 0x2C), scaled2);
    host.widget_vcall_34_set_visible(widget(entry, 8), 0);
    host.widget_vcall_34_set_visible(widget(entry, 4), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0xC), 0);

    at<float>(entry, 0x98) = seconds;
    // JB is taken for "below" AND unordered, so a NaN ends at 2.
    if (seconds >= 3600.0) {
        at<std::int32_t>(entry, 0xA0) = 0;
    } else if (seconds >= 60.0) {
        at<std::int32_t>(entry, 0xA0) = 1;
    } else {
        at<std::int32_t>(entry, 0xA0) = 2;
    }
    release_string(text, host);
}

// 0052C1B0 BSP_HudCounterEntry_Construct
// __thiscall(ECX = entry; void* root, const char* const* names), RET 8;
// EAX = entry. EBP frame, EH frame 00C6BAE2 (the name temporaries).
// Zeroes +70h/+74h/+78h/+7Ch/+80h and +88h/+8Ch/+90h/+94h; byte +0A4h = 0;
// +9Ch = 3; bytes +0A5h/+0A6h/+0A8h/+0A7h/+0A9h = 0. Then twelve children,
// each through a temporary native string (0041E870) passed to 00AA7E00(root
// or +0h, &name, 1) and released: +0h = find(root, names[0]); then under +0h:
// +4h names[1], +8h names[3], +0Ch names[2], +10h names[4], +14h names[5],
// +18h names[8], +1Ch names[10], +20h names[9], +2Ch names[11], +24h
// names[6], +28h names[7] (that order). Then +3Ch..+44h = 00AA67F0(+4h),
// +30h..+38h = 00AA67F0(+8h), +60h/+64h = 00AA6740(+0Ch), +68h/+6Ch =
// 00AA6740(+18h), +48h..+50h = 00AA67F0(+18h), +54h..+5Ch = 00AA67F0(+24h).
// Last, +0h->+34h is called with NO pushed argument (0052C61B-0052C622): the
// callee takes [ESP], which is the caller's saved EDI (the prologue's last
// push). The only caller 0052CBF0 has EDI = its own ECX (the counters
// screen) at 0052CD2A, so the widget gets that pointer as its argument; the
// mis-balanced RET 4 is absorbed by MOV ESP, EBP.
void* cc13_sweepB4_hud_counter_entry_construct_0052c1b0(void* entry, void* root, const char* const* names,
                                                        std::int32_t caller_edi, Cc13SweepB4Host& host) {
    at<std::uint32_t>(entry, 0x70) = 0;
    at<std::uint32_t>(entry, 0x74) = 0;
    at<std::uint32_t>(entry, 0x78) = 0;
    at<std::uint32_t>(entry, 0x7C) = 0;
    at<std::uint32_t>(entry, 0x80) = 0;
    at<std::uint32_t>(entry, 0x88) = 0;
    at<std::uint32_t>(entry, 0x8C) = 0;
    at<std::uint32_t>(entry, 0x90) = 0;
    at<std::uint32_t>(entry, 0x94) = 0;
    at<std::uint8_t>(entry, 0xA4) = 0;
    at<std::int32_t>(entry, 0x9C) = 3;
    at<std::uint8_t>(entry, 0xA5) = 0;
    at<std::uint8_t>(entry, 0xA6) = 0;
    at<std::uint8_t>(entry, 0xA8) = 0;
    at<std::uint8_t>(entry, 0xA7) = 0;
    at<std::uint8_t>(entry, 0xA9) = 0;

    auto find = [&](void* parent, std::size_t field, int name_index) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, names[name_index]);
        at<void*>(entry, field) = host.widget_find_child_00aa7e00(parent, &name, 1);
        release_string(name, host);
    };
    find(root, 0x00, 0);
    find(at<void*>(entry, 0), 0x04, 1);
    find(at<void*>(entry, 0), 0x08, 3);
    find(at<void*>(entry, 0), 0x0C, 2);
    find(at<void*>(entry, 0), 0x10, 4);
    find(at<void*>(entry, 0), 0x14, 5);
    find(at<void*>(entry, 0), 0x18, 8);
    find(at<void*>(entry, 0), 0x1C, 10);
    find(at<void*>(entry, 0), 0x20, 9);
    find(at<void*>(entry, 0), 0x2C, 11);
    find(at<void*>(entry, 0), 0x24, 6);
    find(at<void*>(entry, 0), 0x28, 7);

    auto copy3 = [&](std::size_t field, const float* source) {
        at<float>(entry, field) = source[0];
        at<float>(entry, field + 4) = source[1];
        at<float>(entry, field + 8) = source[2];
    };
    auto copy2 = [&](std::size_t field, const float* source) {
        at<float>(entry, field) = source[0];
        at<float>(entry, field + 4) = source[1];
    };
    copy3(0x3C, host.gui_widget_get_local_position_00aa67f0(widget(entry, 4)));
    copy3(0x30, host.gui_widget_get_local_position_00aa67f0(widget(entry, 8)));
    copy2(0x60, host.gui_widget_get_size_00aa6740(widget(entry, 0xC)));
    copy2(0x68, host.gui_widget_get_size_00aa6740(widget(entry, 0x18)));
    copy3(0x48, host.gui_widget_get_local_position_00aa67f0(widget(entry, 0x18)));
    copy3(0x54, host.gui_widget_get_local_position_00aa67f0(widget(entry, 0x24)));
    host.widget_vcall_34_set_visible(widget(entry, 0), caller_edi);
    return entry;
}

// 00529D70 BSP_HudCounter_LayoutTimer
// __fastcall(ECX = entry; native string a, native string b), RET 10h.
// EH frame 00C6B7E0 (state 1 = both string arguments live).
// +9Ch = 1; 00AA6820(+0h, 1), +0h->+34h(1), 00AA6820(+0h, 0); +18h/+1Ch/+20h/
// +10h hidden. +88h = a; 00AA78D0(+24h, 0.0f); +24h->+58h({0.5, 0.5}); dword
// +24h.+100h = 1; byte +0FCh = 0; 00ABAED0(+24h, &+88h, 1). +14h hidden. +90h
// = b and the same on +28h. scale = the larger text +114h / 960.0 (no 0.1
// floor here); s = (float)(scale + 0.01 (double 00D7A358)) -> +24h->+58h({s,
// 0.5}) and +28h->+58h({s, 0.5}). +30h = s, +54h = 0.0f; 00AA7DC0 on +8h and
// +2Ch; +0Ch->+48h({counter_fraction, 1.0f}); P = 00AA6750(+2Ch), Q =
// 00AA6750(+0Ch); 00AA8240(+2Ch, {(float)(Q.x - 0.02), P.y, P.z});
// +2Ch->+58h({s, 00AA6740(+2Ch)[1]}); +8h/+4h/+0Ch hidden. a then b released.
void cc13_sweepB4_hud_counter_layout_timer_00529d70(void* entry, Cc13SweepBNativeString a,
                                                    Cc13SweepBNativeString b, Cc13SweepB4Host& host) {
    at<std::int32_t>(entry, 0x9C) = 1;
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 1);
    host.widget_vcall_34_set_visible(widget(entry, 0), 1);
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x18), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x1C), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x20), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x10), 0);

    const float half[2] = {0.5f, 0.5f};
    assign_string(entry, 0x88, a, host);
    host.widget_set_local_x_00aa78d0(widget(entry, 0x24), 0.0f);
    host.widget_vcall_58_set_pair(widget(entry, 0x24), half);
    at<std::int32_t>(widget(entry, 0x24), 0x100) = 1;
    at<std::uint8_t>(widget(entry, 0x24), 0xFC) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x24), static_cast<std::uint8_t*>(entry) + 0x88, 1);
    host.widget_vcall_34_set_visible(widget(entry, 0x14), 0);

    assign_string(entry, 0x90, b, host);
    host.widget_set_local_x_00aa78d0(widget(entry, 0x28), 0.0f);
    host.widget_vcall_58_set_pair(widget(entry, 0x28), half);
    at<std::int32_t>(widget(entry, 0x28), 0x100) = 1;
    at<std::uint8_t>(widget(entry, 0x28), 0xFC) = 0;
    host.gui_text_set_localised_source_00abaed0(widget(entry, 0x28), static_cast<std::uint8_t*>(entry) + 0x90, 1);

    const float right = static_cast<float>(at<float>(widget(entry, 0x28), 0x114) / 960.0);
    const float left = static_cast<float>(at<float>(widget(entry, 0x24), 0x114) / 960.0);
    const float scale = left > right ? left : right;
    const float width = static_cast<float>(scale + f64_bits(0x3F847AE140000000ull));
    const float pair24[2] = {width, 0.5f};
    host.widget_vcall_58_set_pair(widget(entry, 0x24), pair24);
    const float pair28[2] = {width, 0.5f};
    host.widget_vcall_58_set_pair(widget(entry, 0x28), pair28);

    float* position = &at<float>(entry, 0x30);
    *position = width;
    at<float>(entry, 0x54) = 0.0f;
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 8), position);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 0x2C), position);
    const float fraction[2] = {counter_fraction(entry), 1.0f};
    host.widget_vcall_48_set_pair(widget(entry, 0xC), fraction);

    float p[3];
    host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p);
    float q_out[3];
    const float* q = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q_out);
    const float qx = q[0];
    const float moved[3] = {static_cast<float>(qx - f64_bits(0x3F947AE140000000ull)), p[1], p[2]};
    host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved);
    const float size[2] = {width, host.gui_widget_get_size_00aa6740(widget(entry, 0x2C))[1]};
    host.widget_vcall_58_set_pair(widget(entry, 0x2C), size);
    host.widget_vcall_34_set_visible(widget(entry, 8), 0);
    host.widget_vcall_34_set_visible(widget(entry, 4), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0xC), 0);
    release_string(a, host);
    release_string(b, host);
}

namespace {

// The 18h-byte observer record built on the stack by 0052BB10 (vtables
// 00CE74FC base / 00CE75CC derived, kept as image-address values).
struct CounterUnitRecord {
    std::uint32_t vtable;
    std::uint32_t f04, f08, f0c;
    std::uint8_t f10;
    std::uint8_t pad[3];
    void* target;  // +14h
};
static_assert(sizeof(void*) != 4 || sizeof(CounterUnitRecord) == 0x18, "18h-byte record on Win32");

// entry+70h: the 18h-record array {data +0h, count +4h}; record i's target at
// data + i*18h + 14h.
void* counter_unit_target(void* entry, std::int32_t index) {
    return at<void*>(at<void*>(entry, 0x70), static_cast<std::size_t>(index) * 0x18 + 0x14);
}

}  // namespace

// 0052BB10 BSP_HudCounter_LayoutScore
// __fastcall(ECX = entry; const array* units, native string text), RET 0Ch.
// EH frame 00C6B9E0 (state 0 = text live; 1..3 the stack record).
// +9Ch = 0; 00AA6820(+0h, 1), +0h->+34h(1), 00AA6820(+0h, 0); +14h/+28h
// hidden; +24h->+58h({0.5, 0.5}), dword +24h.+100h = 0; 0052AE60(+70h, 0);
// +84h = 0.0f. For each units[i] (the count re-read each pass) that is
// non-null: a stack record {00CE74FC, 0, 0, 0, byte 1, unit}, 00694A60(unit,
// &record) [unit non-null], vtable 00CE75CC, 0052AD50(+70h, &record), vtable
// 00CE74FC, 006952A0(record.target, &record) when the target is non-null,
// 00695870(&record). Then units[i]->+5Ch(4): +84h = (float)(target(i)+36Ch +
// +84h) with target(i) read from the +70h array at the SAME index i; else
// +5Ch(18h): for k in 0..target(i)+3CCh-1, slots k 0..4 (unsigned) that are
// non-null add their +370h (the compiler unrolled this by four).
// Then +10h shown; +10h->+88h(target(0)+58h, 0, 1.0f) (no count check);
// text non-empty -> 00ABAED0(+24h, &text, 1), else 00ABBE50(+24h,
// target(0)->+14h(), 1). scale = (float)(+24h.+114h / 960.0) with the 0.1
// floor; +30h = (float)(+54h + scale); 00AA7DC0 on +8h and +2Ch; +0Ch->
// +48h({counter_fraction, 1.0f}); P = 00AA6750(+2Ch), Q = 00AA6750(+0Ch);
// 00AA8240(+2Ch, {(float)(Q.x - 0.02), (float)(P.y - 0.002 (double
// 00CED0E0)), P.z}); +2Ch->+58h({(float)(scale + 0.07 (double 00CED0D8)),
// 00AA6740(+2Ch)[1]}); +8h/+4h/+0Ch hidden; text released.
void cc13_sweepB4_hud_counter_layout_score_0052bb10(void* entry, const void* units,
                                                    Cc13SweepBNativeString text, Cc13SweepB4Host& host) {
    void* const list = static_cast<std::uint8_t*>(entry) + 0x70;
    at<std::int32_t>(entry, 0x9C) = 0;
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 1);
    host.widget_vcall_34_set_visible(widget(entry, 0), 1);
    host.gui_widget_set_visibility_recursive_00aa6820(widget(entry, 0), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x14), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0x28), 0);
    const float half[2] = {0.5f, 0.5f};
    host.widget_vcall_58_set_pair(widget(entry, 0x24), half);
    at<std::int32_t>(widget(entry, 0x24), 0x100) = 0;
    host.counter_units_resize_0052ae60(list, 0);
    at<float>(entry, 0x84) = 0.0f;

    void* units_mut = const_cast<void*>(units);
    for (std::int32_t i = 0; i < at<std::int32_t>(units_mut, 4); ++i) {
        void* unit = at<void**>(units_mut, 0)[i];
        if (unit == nullptr) {
            continue;
        }
        CounterUnitRecord record{};
        record.vtable = 0x00CE74FCu;
        record.f10 = 1;
        record.target = unit;
        if (unit != nullptr) {
            host.observer_register_pair_00694a60(unit, &record);
        }
        record.vtable = 0x00CE75CCu;
        host.counter_units_push_back_0052ad50(list, &record);
        record.vtable = 0x00CE74FCu;
        if (record.target != nullptr) {
            host.observer_unregister_pair_006952a0(record.target, &record);
        }
        host.callback_owner_destroy_00695870(&record);

        if (host.object_vcall_5c_is_kind(at<void**>(units_mut, 0)[i], 4)) {
            const float value = at<float>(counter_unit_target(entry, i), 0x36C);
            at<float>(entry, 0x84) = static_cast<float>(static_cast<double>(value) + at<float>(entry, 0x84));
        } else if (host.object_vcall_5c_is_kind(at<void**>(units_mut, 0)[i], 0x18)) {
            void* target = counter_unit_target(entry, i);
            const std::int32_t count = at<std::int32_t>(target, 0x3CC);
            for (std::int32_t k = 0; k < count; ++k) {
                if (static_cast<std::uint32_t>(k) > 4u) {
                    continue;
                }
                void* slot = at<void*>(target, 0x3D0 + static_cast<std::size_t>(k) * 4);
                if (slot != nullptr) {
                    const float value = at<float>(slot, 0x370);
                    at<float>(entry, 0x84) = static_cast<float>(static_cast<double>(value) + at<float>(entry, 0x84));
                }
            }
        }
    }

    host.widget_vcall_34_set_visible(widget(entry, 0x10), 1);
    host.widget_vcall_88(widget(entry, 0x10), at<std::int32_t>(counter_unit_target(entry, 0), 0x58), 0, 1.0f);
    if (text.length != 0) {
        host.gui_text_set_localised_source_00abaed0(widget(entry, 0x24), &text, 1);
    } else {
        host.gui_text_set_localised_cstring_00abbe50(widget(entry, 0x24),
                                                     host.object_vcall_14_name(counter_unit_target(entry, 0)), 1);
    }
    float scale = static_cast<float>(at<float>(widget(entry, 0x24), 0x114) / 960.0);
    if (!(scale > f64_bits(0x3FB99999A0000000ull))) {
        scale = 0.1f;
    }
    float* position = &at<float>(entry, 0x30);
    *position = static_cast<float>(static_cast<double>(at<float>(entry, 0x54)) + scale);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 8), position);
    host.gui_widget_set_local_position_00aa7dc0(widget(entry, 0x2C), position);
    const float fraction[2] = {counter_fraction(entry), 1.0f};
    host.widget_vcall_48_set_pair(widget(entry, 0xC), fraction);

    float p[3];
    host.widget_get_resolved_position_00aa6750(widget(entry, 0x2C), p);
    const float py = p[1];
    float q_out[3];
    const float* q = host.widget_get_resolved_position_00aa6750(widget(entry, 0xC), q_out);
    const float qx = q[0];
    const float moved[3] = {static_cast<float>(qx - f64_bits(0x3F947AE140000000ull)),
                            static_cast<float>(py - f64_bits(0x3F60624DE0000000ull)), p[2]};
    host.widget_set_resolved_position_00aa8240(widget(entry, 0x2C), moved);
    const float height = host.gui_widget_get_size_00aa6740(widget(entry, 0x2C))[1];
    const float size[2] = {static_cast<float>(scale + f64_bits(0x3FB1EB8520000000ull)), height};
    host.widget_vcall_58_set_pair(widget(entry, 0x2C), size);
    host.widget_vcall_34_set_visible(widget(entry, 8), 0);
    host.widget_vcall_34_set_visible(widget(entry, 4), 0);
    host.widget_vcall_34_set_visible(widget(entry, 0xC), 0);
    release_string(text, host);
}

// 0052B080 BSP_HudCounter_Tick
// __thiscall(ECX = entry; float seconds), RET 4. EH frame 00C6B8C8 (the two
// by-value string copies of mode 1).
// Mode +9Ch:
// 2: unless +0h->+38h() (visible) or any byte +0A6h/+0A5h/+0A8h/+0A7h/+0A9h
//    is set or 004B6260([00E188A8] + 1EF0h) answers true, nothing; else
//    00528B00(entry, seconds).
// 1: when +0h->+38h(): b' = copy of +90h, a' = copy of +88h (00426060, in
//    that order), then 0052A470(entry, a', b') (which releases them).
// 0: when +0h->+38h(): over the +70h records (count +74h re-read): a null
//    target -> 0052ADD0(+70h, i) and the same index again; else target->
//    +5Ch(6) adds (+2Ch * +34h) + (+30h * +38h) of target+0A20h (the getters
//    00939F70 +38h, 00939F80 +34h, 00939FC0 +30h, 00939FB0 +2Ch, called in
//    that order) to sumA; then +5Ch(4) adds target+370h to sumB, else +5Ch(18h)
//    adds slot k's +370h for k in 0..+3CCh-1 where slots past 4 are NULL and
//    STILL dereferenced (00000370h faults in the image as here), and slots
//    0..4 are not null-checked. sumB lives in the seconds argument slot.
//    Then count 0 -> +0h->+34h(0); else 00528E90(entry, (float)(sumB / +84h),
//    (float)(sumA / +84h)).
// Other modes: nothing. No RNG draws.
void cc13_sweepB4_hud_counter_tick_0052b080(void* entry, float seconds, Cc13SweepB4Host& host) {
    const std::int32_t mode = at<std::int32_t>(entry, 0x9C);
    if (mode == 2) {
        if (!host.widget_vcall_38_is_visible(widget(entry, 0)) && at<std::uint8_t>(entry, 0xA6) == 0 &&
            at<std::uint8_t>(entry, 0xA5) == 0 && at<std::uint8_t>(entry, 0xA8) == 0 &&
            at<std::uint8_t>(entry, 0xA7) == 0 && at<std::uint8_t>(entry, 0xA9) == 0 &&
            !host.session_counts_mission_start_004b6260(static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x1EF0)) {
            return;
        }
        host.hud_counter_format_time_00528b00(entry, seconds);
        return;
    }
    if (mode == 1) {
        if (!host.widget_vcall_38_is_visible(widget(entry, 0))) {
            return;
        }
        Cc13SweepBNativeString b_copy;
        host.native_string_copy_construct_00426060(&b_copy, static_cast<std::uint8_t*>(entry) + 0x90);
        Cc13SweepBNativeString a_copy;
        host.native_string_copy_construct_00426060(&a_copy, static_cast<std::uint8_t*>(entry) + 0x88);
        cc13_sweepB4_hud_counter_layout_text_pair_0052a470(entry, a_copy, b_copy, host);
        return;
    }
    if (mode != 0) {
        return;
    }
    if (!host.widget_vcall_38_is_visible(widget(entry, 0))) {
        return;
    }
    float sum_a = 0.0f;
    float sum_b = 0.0f;
    void* const list = static_cast<std::uint8_t*>(entry) + 0x70;
    std::int32_t i = 0;
    while (i < at<std::int32_t>(entry, 0x74)) {
        if (counter_unit_target(entry, i) == nullptr) {
            host.counter_units_remove_at_0052add0(list, i);
            continue;
        }
        if (host.object_vcall_5c_is_kind(counter_unit_target(entry, i), 6)) {
            void* motion = static_cast<std::uint8_t*>(counter_unit_target(entry, i)) + 0xA20;
            const float g38 = static_cast<float>(host.getter_38_00939f70(motion));
            const float g34 = static_cast<float>(host.getter_34_00939f80(motion));
            const float g30 = static_cast<float>(host.getter_30_00939fc0(motion));
            const double g2c = host.getter_2c_00939fb0(motion);
            sum_a = static_cast<float>((g2c * g34 + static_cast<double>(g30) * g38) + sum_a);
        }
        if (host.object_vcall_5c_is_kind(counter_unit_target(entry, i), 4)) {
            sum_b = static_cast<float>(static_cast<double>(at<float>(counter_unit_target(entry, i), 0x370)) + sum_b);
        } else if (host.object_vcall_5c_is_kind(counter_unit_target(entry, i), 0x18)) {
            void* target = counter_unit_target(entry, i);
            const std::int32_t count = at<std::int32_t>(target, 0x3CC);
            for (std::int32_t k = 0; k < count; ++k) {
                void* slot = static_cast<std::uint32_t>(k) <= 4u
                                 ? at<void*>(target, 0x3D0 + static_cast<std::size_t>(k) * 4)
                                 : nullptr;
                // No null check in the image: a null slot reads 00000370h.
                sum_b = static_cast<float>(static_cast<double>(at<float>(slot, 0x370)) + sum_b);
            }
        }
        ++i;
    }
    if (at<std::int32_t>(entry, 0x74) == 0) {
        host.widget_vcall_34_set_visible(widget(entry, 0), 0);
        return;
    }
    const float total = at<float>(entry, 0x84);
    const float share_a = static_cast<float>(static_cast<double>(sum_a) / total);
    const float share_b = static_cast<float>(static_cast<double>(sum_b) / total);
    host.hud_counter_update_bar_00528e90(entry, share_b, share_a);
}

namespace {

// A by-value std::list {proxy, head, size}; filled by 005AAAF0, emptied by 005A0B80.
struct ListCopy {
    void* words[3];
};

void* list_head(ListCopy& list) {
    return list.words[1];
}

// The first element of a list copy through the inlined checked begin()
// dereference (head->next == head calls 00BF6713): node+8h.
void* list_first_value(ListCopy& list, Cc13SweepB2Host& host) {
    void* head = list_head(list);
    void* first = *static_cast<void**>(head);
    if (first == head) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    return at<void*>(first, 8);
}

// _ftol2 (00BF7420) on an ST0 value: truncation, 80000000h when out of range.
std::int32_t ftol_trunc(double value) {
    if (!(value > -2147483649.0 && value < 2147483648.0)) {
        return static_cast<std::int32_t>(0x80000000u);
    }
    return static_cast<std::int32_t>(value);
}

}  // namespace

// ---------------------------------------------------------------------------
// 00671D60 BSP_HudSupportManager_ShowSelectionInfo (hypothesis)
// __fastcall(ECX = screen), RET 0. EH frame (handler 00C7BFC2) guards the
// list copies and the native-string temporaries; not modelled.
// Screen: +0DCh[4] page widgets indexed by +324h; +0F0h/+0F4h widgets; +170h
// name text; +1A8h points text; +1F4h; +300h/+304h checked iterator into the
// unit list (node+8h = unit); +34Ch the support map's size and +350h/+354h a
// checked iterator into it, whose value (node+8h) is {object +0, std::list +4,
// int +10h}. Every dereference goes through the inlined debug checks.
//
// 00670020 rebuild; page widget +34h(0). 00671D9E-00671E10: when the map is
// non-empty, a full copy of the current value (object, 005AAAF0 list copy,
// int) is taken and dropped (005A0B80) just to test object != 0. Set: the
// name text gets object->+14h() (00ABBE50 with 1). Clear: 00671E9A-00671F97
// tests (map non-empty, first list element's +3Ch non-null, and +3Ch's
// +5Ch(9)) on two more list copies, dropped in reverse order; true -> the
// name text gets (+3Ch)->+14h(). Otherwise, map non-empty: the first list
// element's +15Ch non-zero (SETA) -> the value through 00658B50, a fourth
// list copy, its begin() through 0059FB60, and that element's +14h() name;
// +15Ch zero -> "globals.unitclass_spawnpoint". Map empty: the current unit's
// +14h() name.
// Tail 00672109: 0065BBC0, +5Ch = 1, +0F0h->+34h(1), +0F4h->+34h(0). With
// +1F4h set and 004BCA50([00E188A8]) == 8: a unit that is neither kind 9 nor
// 45h gives "ingame.sm_slots|.: " + 008434D0(*00658A60()) (the string is
// built BEFORE 00658A60 runs); otherwise "ingame.sm_slots|.: " +
// 006BDC30(unit ? unit + (kind 45h ? 72Ch : 1188h) : 0) (00658A60 runs BEFORE
// the string). Else "ingame.sm_cp|.: " + int(_ftol2(00946F00([00E188A8] +
// 18ECh))), the number string built first. Each goes to 00ABAED0(points text,
// s, 1), and the temporaries are released newest first.
// No RNG draws.
void cc13_sweepB4_hud_support_show_selection_info_00671d60(void* screen, Cc13SweepB4Host& host) {
    auto map_check = [&]() {
        check_deref(at<void*>(screen, 0x350), at<void*>(screen, 0x354), host);
    };
    auto map_node = [&]() { return static_cast<std::uint8_t*>(at<void*>(screen, 0x354)); };
    void* const name_text = at<void*>(screen, 0x170);

    host.hud_support_rebuild_00670020(screen);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xDC + at<std::int32_t>(screen, 0x324) * 4), 0);

    bool has_object = false;
    if (at<std::int32_t>(screen, 0x34C) != 0) {
        map_check();
        std::uint8_t* node = map_node();
        void* object = at<void*>(node, 8);
        ListCopy copy;
        host.list_copy_005aaaf0(&copy, node + 0xC);
        (void)at<std::uint32_t>(node, 0x18);  // copied into the by-value pair, unused
        has_object = object != nullptr;
        host.list_tidy_005a0b80(&copy);
    }

    if (has_object) {
        map_check();
        std::uint8_t* node = map_node();
        void* object = at<void*>(node, 8);
        ListCopy copy;
        host.list_copy_005aaaf0(&copy, node + 0xC);
        (void)at<std::uint32_t>(node, 0x18);
        host.gui_text_set_localised_cstring_00abbe50(name_text, host.object_vcall_14_name(object), 1);
        host.list_tidy_005a0b80(&copy);
    } else {
        bool kind9 = false;
        if (at<std::int32_t>(screen, 0x34C) != 0) {
            map_check();
            ListCopy first_copy;
            host.list_copy_005aaaf0(&first_copy, map_node() + 0xC);
            void* element = list_first_value(first_copy, host);
            if (at<void*>(element, 0x3C) != nullptr) {
                map_check();
                ListCopy second_copy;
                host.list_copy_005aaaf0(&second_copy, map_node() + 0xC);
                void* second = list_first_value(second_copy, host);
                kind9 = host.object_vcall_5c_is_kind(at<void*>(second, 0x3C), 9);
                host.list_tidy_005a0b80(&second_copy);
            }
            host.list_tidy_005a0b80(&first_copy);
        }

        if (kind9) {
            map_check();
            ListCopy copy;
            host.list_copy_005aaaf0(&copy, map_node() + 0xC);
            void* element = list_first_value(copy, host);
            host.gui_text_set_localised_cstring_00abbe50(
                name_text, host.object_vcall_14_name(at<void*>(element, 0x3C)), 1);
            host.list_tidy_005a0b80(&copy);
        } else if (at<std::int32_t>(screen, 0x34C) != 0) {
            void* iterator = static_cast<std::uint8_t*>(screen) + 0x350;
            map_check();
            ListCopy copy;
            host.list_copy_005aaaf0(&copy, map_node() + 0xC);
            void* element = list_first_value(copy, host);
            const bool has_class = at<std::uint32_t>(element, 0x15C) > 0;
            host.list_tidy_005a0b80(&copy);
            if (has_class) {
                void* value = host.support_map_iterator_value_00658b50(iterator);
                ListCopy value_copy;
                host.list_copy_005aaaf0(&value_copy, static_cast<std::uint8_t*>(value) + 4);
                void* list_iterator[2] = {&value_copy, *static_cast<void**>(list_head(value_copy))};
                void* object = *host.list_iterator_value_0059fb60(list_iterator);
                host.gui_text_set_localised_cstring_00abbe50(name_text, host.object_vcall_14_name(object), 1);
                host.list_tidy_005a0b80(&value_copy);
            } else {
                host.gui_text_set_localised_cstring_00abbe50(name_text, "globals.unitclass_spawnpoint", 1);
            }
        } else {
            check_deref(at<void*>(screen, 0x300), at<void*>(screen, 0x304), host);
            void* unit = at<void*>(at<void*>(screen, 0x304), 8);
            host.gui_text_set_localised_cstring_00abbe50(name_text, host.object_vcall_14_name(unit), 1);
        }
    }

    // 00672109
    host.hud_support_layout_name_label_0065bbc0(screen);
    at<std::int32_t>(screen, 0x5C) = 1;
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xF0), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xF4), 0);
    void* const points_text = at<void*>(screen, 0x1A8);

    if (at<std::int32_t>(screen, 0x1F4) != 0 && host.game_effective_mode_004bca50(host.global_00e188a8()) == 8) {
        void* iterator = static_cast<std::uint8_t*>(screen) + 0x300;
        auto unit_check = [&]() { check_deref(at<void*>(iterator, 0), at<void*>(iterator, 4), host); };
        auto current_unit = [&]() { return at<void*>(at<void*>(iterator, 4), 8); };
        unit_check();
        bool air = host.object_vcall_5c_is_kind(current_unit(), 9);
        if (!air) {
            unit_check();
            air = host.object_vcall_5c_is_kind(current_unit(), 0x45);
        }
        if (!air) {
            Cc13SweepBNativeString prefix;
            host.native_string_assign_0041e870(&prefix, "ingame.sm_slots|.: ");
            void* shipyard = *host.support_unit_iterator_value_00658a60(iterator);
            const std::int32_t idle = host.shipyard_count_idle_slots_008434d0(shipyard);
            Cc13SweepBNativeString text;
            host.native_string_concat_int_004263b0(&prefix, &text, idle);
            host.gui_text_set_localised_source_00abaed0(points_text, &text, 1);
            release_string(text, host);
            release_string(prefix, host);
            return;
        }
        // 00672243
        unit_check();
        const bool carrier = host.object_vcall_5c_is_kind(current_unit(), 0x45);
        void* unit = *host.support_unit_iterator_value_00658a60(iterator);
        void* block = nullptr;
        if (unit != nullptr) {
            block = static_cast<std::uint8_t*>(unit) + (carrier ? 0x72C : 0x1188);
        }
        Cc13SweepBNativeString prefix;
        host.native_string_assign_0041e870(&prefix, "ingame.sm_slots|.: ");
        const std::int32_t idle = host.air_ops_count_idle_slots_006bdc30(block);
        Cc13SweepBNativeString text;
        host.native_string_concat_int_004263b0(&prefix, &text, idle);
        host.gui_text_set_localised_source_00abaed0(points_text, &text, 1);
        release_string(text, host);
        release_string(prefix, host);
        return;
    }

    // 00672315
    void* player = at<void*>(host.global_00e188a8(), 0x18EC);
    const std::int32_t points = ftol_trunc(host.player_support_points_00946f00(player));
    Cc13SweepBNativeString number;
    host.native_string_from_int_004260b0(&number, points);
    Cc13SweepBNativeString prefix;
    host.native_string_assign_0041e870(&prefix, "ingame.sm_cp|.: ");
    Cc13SweepBNativeString text;
    host.native_string_concat_004261a0(&prefix, &text, &number);
    host.gui_text_set_localised_source_00abaed0(points_text, &text, 1);
    release_string(text, host);
    release_string(prefix, host);
    release_string(number, host);
}

// ---------------------------------------------------------------------------
// 005987F0 BSP_MainMenuScreen_Enter
// __thiscall(ECX = screen), RET 0; main-menu screen vtable +18h (no direct
// callers). EH frame 00C712D8 (the string temporaries).
// [00E198B0] = 0; 0076FAD0([00E188A8] + 1EF0h, 0); byte [00E188A8]+218Ch = 0;
// bytes +2C4h/+5Ch = 0. A temporary "" (00CE3A0C: {0,0}, 0041DD40(0, 1),
// memcpy of length + 1 when data is non-null) goes to 00518D60(0, 0, 0, &s)
// and is released. 0058D9E0(screen, 0). srand((unsigned)_time64(0)) (only
// EAX). +58h = 0; +44h..+54h = 1..5; twenty swaps of +44h[rand() % 5] and
// +44h[rand() % 5] (the first rand is the first index) -- the CRT generator,
// NOT the shared 00BD2F10. 00AAC870(+340h). ".bik" (00CEC130, built like "")
// = U; N = 004260B0(+44h[+58h]); P = 0043C130("movies\\FE_bg_0" (00CEF7C0),
// &N); C = 004261A0(&P, &U); 00AAE320(+340h, &C, 0); C, P, N, U released;
// 00AB0EB0(+340h). Page by [00E08874]: 1 -> 00584AE0(0), 2 -> 00584F50, 3 ->
// 005853C0, 4..8 -> 00597870(value), 9 -> 0058C010, 0Bh -> 005886F0, 0Ch ->
// 00594BF0, other values none. +130h = 1.0f; +184h/+188h/+18Ch = 0.0f;
// 00549620([00E1930C], 0); [00E198B8] non-null with +40h non-null ->
// 005F6EF0(+40h).
void cc13_sweepB4_main_menu_enter_005987f0(void* screen, Cc13SweepB4Host& host) {
    auto image_dword = [&](std::uint32_t address) -> std::uint32_t& {
        return *static_cast<std::uint32_t*>(host.image_global(address));
    };
    // The inlined native string of a C string literal.
    auto literal = [&](Cc13SweepBNativeString& text, std::uint32_t length, const void* bytes) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, bytes, text.length + 1);
        }
    };

    image_dword(0x00E198B0) = 0;
    host.session_set_mode_0076fad0(static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x1EF0, 0);
    at<std::uint8_t>(host.global_00e188a8(), 0x218C) = 0;
    at<std::uint8_t>(screen, 0x2C4) = 0;
    at<std::uint8_t>(screen, 0x5C) = 0;
    Cc13SweepBNativeString title;
    literal(title, 0, host.image_global(0x00CE3A0C));
    host.frontend_frame_select_00518d60(0, 0, 0, &title);
    release_string(title, host);

    host.main_menu_build_mission_map_icons_0058d9e0(screen, 0);
    host.crt_srand_00bf98c2(static_cast<std::uint32_t>(host.crt_time64_00bf9886(nullptr)));
    at<std::int32_t>(screen, 0x58) = 0;
    for (std::int32_t k = 0; k < 5; ++k) {
        at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(k) * 4) = k + 1;
    }
    for (int pass = 0; pass < 20; ++pass) {
        const std::int32_t i = host.crt_rand_00bf98cf() % 5;
        const std::int32_t j = host.crt_rand_00bf98cf() % 5;
        const std::int32_t held = at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(i) * 4);
        at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(i) * 4) =
            at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(j) * 4);
        at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(j) * 4) = held;
    }

    void* movie = at<void*>(screen, 0x340);
    host.gui_movie_stop_00aac870(movie);
    Cc13SweepBNativeString extension;
    literal(extension, 4, host.image_global(0x00CEC130));
    Cc13SweepBNativeString number;
    host.native_string_from_int_004260b0(
        &number, at<std::int32_t>(screen, 0x44 + static_cast<std::size_t>(at<std::int32_t>(screen, 0x58)) * 4));
    Cc13SweepBNativeString path_prefix;
    host.native_string_prefixed_0043c130(&path_prefix, static_cast<const char*>(host.image_global(0x00CEF7C0)),
                                         &number);
    Cc13SweepBNativeString path;
    host.native_string_concat_004261a0(&path_prefix, &path, &extension);
    host.gui_movie_set_source_00aae320(movie, &path, 0);
    release_string(path, host);
    release_string(path_prefix, host);
    release_string(number, host);
    release_string(extension, host);
    host.gui_movie_start_00ab0eb0(at<void*>(screen, 0x340));

    const std::uint32_t page = image_dword(0x00E08874);
    switch (page) {
    case 1: host.main_menu_build_top_level_00584ae0(screen, 0); break;
    case 2: host.main_menu_build_single_player_00584f50(screen); break;
    case 3: host.main_menu_build_multiplayer_005853c0(screen); break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: host.main_menu_build_mission_list_00597870(screen, static_cast<std::int32_t>(page)); break;
    case 9: host.main_menu_build_mission_detail_0058c010(screen); break;
    case 0xB: host.main_menu_build_tactical_library_005886f0(screen); break;
    case 0xC: host.main_menu_build_objective_rows_00594bf0(screen); break;
    default: break;
    }
    at<float>(screen, 0x130) = 1.0f;
    at<float>(screen, 0x184) = 0.0f;
    at<float>(screen, 0x188) = 0.0f;
    at<float>(screen, 0x18C) = 0.0f;
    host.main_menu_command_owner_set_help_variant_00549620(*static_cast<void**>(host.image_global(0x00E1930C)), 0);
    void* options = *static_cast<void**>(host.image_global(0x00E198B8));
    if (options != nullptr && at<void*>(options, 0x40) != nullptr) {
        host.options_screen_sync_profile_values_005f6ef0(at<void*>(options, 0x40));
    }
}

// ---------------------------------------------------------------------------
// 005E1CF0 BSP_MultiGameLobby_StepSetting
// __thiscall(ECX = screen; int step), RET 4. No EH frame.
// game = [00E188A8]: game+1FE4h == 2 or game+5FCh == 0 -> return (nothing
// else). slot = 005D5630(00A9C920(+228h)). Every path below ends with +10h =
// 2.
// slot 1 (map): 005E1BF0(step); 0041E350(00E195BC, ""), 0041E350(00E195C4,
// ""); +2F8h != -1 and *005D4AF0(+2E0h, +2F8h) == (game+5FCh)+1098h -> +2F0h
// = +2F8h, +2F4h = +2FCh; otherwise +2F4h = +2F0h = -1.
// slot 0Dh: 005D4910 true -> step > 0: +++2F4h, wraps to 0 at 00427AC0(
// 005D5160()+4); step < 0: --+2F4h, below 0 -> size - 1; step 0 nothing.
// slot 5 (00E19570, the value array 00E1955C[5]): n = 00575E80(00576B10()+
// 10h, &5)+8h; value = (value - step + n) % n (unsigned) -> accepted when
// *005C49C0(005C6850(005E1990(), &5), &value) != 0; otherwise up to n' tries
// (n' re-read each pass) of the same step, each accepted when the flag == 1;
// none accepted -> the saved value is restored.
// other slots s (00E1955C[s]): the same with value = (value + n + step) % n.
// No RNG draws.
void cc13_sweepB4_lobby_step_setting_005e1cf0(void* screen, std::int32_t step, Cc13SweepB4Host& host) {
    void* game = host.global_00e188a8();
    if (at<std::int32_t>(game, 0x1FE4) == 2) {
        return;
    }
    if (at<void*>(game, 0x5FC) == nullptr) {
        return;
    }
    const std::int32_t slot =
        host.lobby_slot_for_menu_row_005d5630(host.listbox_selected_index_00a9c920(at<void*>(screen, 0x228)));
    auto option_count = [&](std::int32_t key) -> std::uint32_t {
        void* registry = host.lobby_option_registry_00576b10();
        return at<std::uint32_t>(
            host.lobby_map_subscript_00575e80(static_cast<std::uint8_t*>(registry) + 0x10, &key), 8);
    };
    auto option_flag = [&](std::int32_t key, std::uint32_t value) -> std::uint8_t {
        void* inner = host.lobby_tree_subscript_005c6850(host.lobby_option_table_005e1990(), &key);
        return *host.lobby_flag_subscript_005c49c0(inner, &value);
    };

    if (slot == 1) {
        host.lobby_step_map_index_005e1bf0(screen, step);
        const char* empty = static_cast<const char*>(host.image_global(0x00CE3A0C));
        host.native_string_assign_cstring_0041e350(host.image_global(0x00E195BC), empty);
        host.native_string_assign_cstring_0041e350(host.image_global(0x00E195C4), empty);
        const std::int32_t pending = at<std::int32_t>(screen, 0x2F8);
        bool keep = false;
        if (pending != -1) {
            void* record = host.lobby_slot_record_005d4af0(static_cast<std::uint8_t*>(screen) + 0x2E0, pending);
            void* game_now = host.global_00e188a8();
            keep = at<std::uint32_t>(record, 0) == at<std::uint32_t>(at<void*>(game_now, 0x5FC), 0x1098);
        }
        if (keep) {
            at<std::int32_t>(screen, 0x2F0) = at<std::int32_t>(screen, 0x2F8);
            at<std::int32_t>(screen, 0x2F4) = at<std::int32_t>(screen, 0x2FC);
        } else {
            at<std::int32_t>(screen, 0x2F4) = -1;
            at<std::int32_t>(screen, 0x2F0) = -1;
        }
    } else if (slot == 0xD) {
        if (host.lobby_slot_swap_has_pair_005d4910(screen)) {
            if (step > 0) {
                at<std::int32_t>(screen, 0x2F4) += 1;
                void* names = host.lobby_slot_names_005d5160(screen);
                const std::int32_t size =
                    host.native_string_vector_size_00427ac0(static_cast<std::uint8_t*>(names) + 4);
                if (at<std::int32_t>(screen, 0x2F4) == size) {
                    at<std::int32_t>(screen, 0x2F4) = 0;
                }
            } else if (step < 0) {
                at<std::int32_t>(screen, 0x2F4) -= 1;
                if (at<std::int32_t>(screen, 0x2F4) < 0) {
                    void* names = host.lobby_slot_names_005d5160(screen);
                    at<std::int32_t>(screen, 0x2F4) =
                        host.native_string_vector_size_00427ac0(static_cast<std::uint8_t*>(names) + 4) - 1;
                }
            }
        }
    } else {
        std::uint32_t& value = *static_cast<std::uint32_t*>(host.image_global(0x00E1955C + slot * 4));
        const std::uint32_t saved = value;
        const std::uint32_t n = option_count(slot);
        // slot 5 steps backwards (SUB), every other slot forwards (ADD).
        auto next = [&]() {
            return slot == 5 ? (value - static_cast<std::uint32_t>(step) + n) % n
                             : (value + n + static_cast<std::uint32_t>(step)) % n;
        };
        value = next();
        if (option_flag(slot, value) == 0) {
            bool accepted = false;
            for (std::uint32_t tries = 0; tries < option_count(slot); ++tries) {
                value = next();
                if (option_flag(slot, value) == 1) {
                    accepted = true;
                    break;
                }
            }
            if (!accepted) {
                value = saved;
            }
        }
    }
    at<std::int32_t>(screen, 0x10) = 2;
}

// ---------------------------------------------------------------------------
// 00616C70 BSP_HudPowerupManagerScreen_FillSecondRows
// __thiscall(ECX = screen), RET 0. EBP frame, no EH frame.
// Three checked dword vectors {proxy +0, first +4, last +8, end +0Ch}: A =
// +194h, the source pointer vector S = +184h, B = +1A4h. Every inlined
// checked-iterator test (first <= last, a dereferenced or advanced iterator
// < last) calls 00BF6713 and carries on; the same-container tests compare a
// register with itself and are omitted.
// 1. 004685A0(A, &out, {A, A.first}, {A, A.last}) erases A.
// 2. For each p in S (S.last re-read every pass): rec = *p; push_back(A,
//    rec+8 ? word((rec+8)->+4 ->+174h) : 0) through 00442190.
// 3. byte +200h set: B cleared (the inlined erase(begin, end): memmove_s of
//    the zero-length tail when it is positive, last = first + tail), then A
//    copied into B, then return.
// 4. Otherwise: B's size != A's size (a null first counts 0), or an element
//    differs (B scanned from its first, A in step) -> byte +1E4h = 1 and
//    +1E8h = 1.0f; then B cleared and A copied into B as in 3.
// B's push_back is inlined: B.first non-null and size < capacity -> store
// and last += 4, else 00441F30(B, &out, {B, B.last}, &value).
void cc13_sweepB4_powerup_fill_second_rows_00616c70(void* screen, Cc13SweepB4Host& host) {
    auto* const s = static_cast<std::uint8_t*>(screen);
    void* const a = s + 0x194;
    void* const src = s + 0x184;
    void* const b = s + 0x1A4;
    auto first = [](void* v) -> std::uint32_t*& { return at<std::uint32_t*>(v, 4); };
    auto last = [](void* v) -> std::uint32_t*& { return at<std::uint32_t*>(v, 8); };
    auto end = [](void* v) -> std::uint32_t*& { return at<std::uint32_t*>(v, 0xC); };
    auto check = [&](bool ok) {
        if (!ok) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    auto size_of = [&](void* v) -> std::uint32_t {
        return first(v) == nullptr ? 0u : static_cast<std::uint32_t>(last(v) - first(v));
    };
    // The inlined erase(begin(), end()) of B (00616D60 / 00616F34).
    auto clear_b = [&]() {
        std::uint32_t* const end_it = last(b);
        check(first(b) <= end_it);
        std::uint32_t* const begin_it = first(b);
        check(begin_it <= last(b));
        if (begin_it == end_it) {
            return;
        }
        const std::int32_t tail = static_cast<std::int32_t>(last(b) - end_it);
        std::uint32_t* const new_last = begin_it + tail;
        if (tail > 0) {
            host.crt_memmove_s_00bf67a7(begin_it, static_cast<std::uint32_t>(tail) * 4, end_it,
                                        static_cast<std::uint32_t>(tail) * 4);
        }
        last(b) = new_last;
    };
    // A copied into B by the inlined push_back (00616DAB / 00616F77).
    auto copy_a_to_b = [&]() {
        std::uint32_t* it = first(a);
        check(it <= last(a));
        for (;;) {
            std::uint32_t* const stop = last(a);
            check(first(a) <= stop);
            if (it == stop) {
                return;
            }
            check(it < last(a));
            std::uint32_t* const b_first = first(b);
            const std::uint32_t count = size_of(b);
            if (b_first != nullptr && count < static_cast<std::uint32_t>(end(b) - b_first)) {
                *last(b) = *it;
                last(b) += 1;
            } else {
                std::uint32_t* const where = last(b);
                check(b_first <= where);
                void* out[2];
                host.dword_vector_insert_one_00441f30(b, out, b, where, it);
            }
            check(it < last(a));
            ++it;
        }
    };

    // 1.
    std::uint32_t* const a_last = last(a);
    check(first(a) <= a_last);
    std::uint32_t* const a_first = first(a);
    check(a_first <= last(a));
    void* erase_out[2];
    host.dword_vector_erase_range_004685a0(a, erase_out, a, a_first, a, a_last);

    // 2.
    std::uint32_t* p = first(src);
    check(p <= last(src));
    for (;;) {
        std::uint32_t* const stop = last(src);
        check(first(src) <= stop);
        if (p == stop) {
            break;
        }
        check(p < last(src));
        void* rec = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*p));
        std::uint32_t value = 0;
        if (at<void*>(rec, 8) != nullptr) {
            check(p < last(src));
            void* reread = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*p));
            void* inner = at<void*>(at<void*>(reread, 8), 4);
            value = at<std::uint16_t>(inner, 0x174);
        }
        host.dword_vector_push_back_00442190(a, &value);
        check(p < last(src));
        ++p;
    }

    // 3. / 4.
    if (at<std::uint8_t>(screen, 0x200) == 0) {
        bool changed = size_of(b) != size_of(a);
        if (!changed && first(b) != nullptr && size_of(b) != 0) {
            std::uint32_t* bi = first(b);
            check(bi <= last(b));
            std::uint32_t* ai = first(a);
            check(ai <= last(a));
            for (;;) {
                std::uint32_t* const stop = last(b);
                check(first(b) <= stop);
                if (bi == stop) {
                    break;
                }
                check(bi < last(b));
                check(ai < last(a));
                if (*bi != *ai) {
                    changed = true;
                    break;
                }
                check(bi < last(b));
                ++bi;
                check(ai < last(a));
                ++ai;
            }
        }
        if (changed) {
            at<std::uint8_t>(screen, 0x1E4) = 1;
            at<float>(screen, 0x1E8) = 1.0f;
        }
    }
    clear_b();
    copy_a_to_b();
}

// ---------------------------------------------------------------------------
// 00637080 BSP_HudSceneInitScreen_Enter
// __thiscall(ECX = screen), RET 0; vtable 00CF569C slot +18h (no direct
// callers). EH frame 00C79F41 (the hint temporary and the name strings).
// game = [00E188A8]. 00ABBE50(+20h, game+1FE4h == 0 or byte game+218Ch set ?
// "FE_pc.init_start" : "ingame.init_waitothers", 1); +1Ch->+34h(1). hint =
// &hints[index] when the global vector {first 00E08784, last 00E08788} of
// 8-byte native strings is non-empty (index 00E0877C, checked against the
// size through 00BF6713), else a temporary "" (released after use when it
// was built). Children, each through a temporary name built inline
// (0041DD40(length, 1) + memcpy of length + 1) and released after the
// lookup: 00ABAED0(00AA7E00(+1Ch, "hint_Text", 1), hint, 1); bg =
// 00AA7E00(+1Ch, "bg_Group", 1); title = 00AA7E00(bg, "title_Text", 1);
// 00ABAED0(title, 00E08790, 1); 00AA7E00(bg, "frameFlag_Icon", 1)->+88h(byte
// [00E08798] != 0, 0, 1.0f); logo = 00AA7E00(bg, "loadingLogo_FrameBox", 1).
// s = (float)(title+114h / 960.0); s > 4/15 (double 00CEC3E8, FCOMI + JBE so
// NaN is "not above") -> logo->+58h({(float)(s + 4/15), 00AA6740(logo)[1]}),
// else logo->+58h({0.53333336f (00CEC3E4), 00AA6740(logo)[1]}). Then, game+
// 1FE4h != 0 and byte game+218Ch clear: screen' = [00E19698], bytes +4/+5 =
// 1, 004F83B0(screen'), screen'->+18h().
void cc13_sweepB4_hud_scene_init_enter_00637080(void* screen, Cc13SweepB4Host& host) {
    auto literal = [&](Cc13SweepBNativeString& text, std::uint32_t length, std::uint32_t address) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, host.image_global(address), text.length + 1);
        }
    };
    auto find = [&](void* parent, std::uint32_t length, std::uint32_t address) {
        Cc13SweepBNativeString name;
        literal(name, length, address);
        void* child = host.widget_find_child_00aa7e00(parent, &name, 1);
        release_string(name, host);
        return child;
    };

    void* game = host.global_00e188a8();
    const bool front_end = at<std::int32_t>(game, 0x1FE4) == 0 || at<std::uint8_t>(game, 0x218C) != 0;
    host.gui_text_set_localised_cstring_00abbe50(
        at<void*>(screen, 0x20),
        static_cast<const char*>(host.image_global(front_end ? 0x00CF56ECu : 0x00CF56D4u)), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x1C), 1);

    const std::uint8_t* hints_first = *static_cast<std::uint8_t**>(host.image_global(0x00E08784));
    const std::uint8_t* hints_last = *static_cast<std::uint8_t**>(host.image_global(0x00E08788));
    const void* hint;
    Cc13SweepBNativeString empty;
    bool built_empty = false;
    if (hints_first != nullptr && (hints_last - hints_first) / 8 != 0) {
        const std::uint32_t index = *static_cast<std::uint32_t*>(host.image_global(0x00E0877C));
        if (!(index < static_cast<std::uint32_t>((hints_last - hints_first) / 8))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        hint = *static_cast<std::uint8_t**>(host.image_global(0x00E08784)) + static_cast<std::size_t>(index) * 8;
    } else {
        host.native_string_assign_0041e870(&empty, static_cast<const char*>(host.image_global(0x00CE3A0C)));
        hint = &empty;
        built_empty = true;
    }
    {
        Cc13SweepBNativeString name;
        literal(name, 9, 0x00CEF2F4);  // "hint_Text"
        void* hint_text = host.widget_find_child_00aa7e00(at<void*>(screen, 0x1C), &name, 1);
        host.gui_text_set_localised_source_00abaed0(hint_text, hint, 1);
        release_string(name, host);
    }
    if (built_empty) {
        release_string(empty, host);
    }
    void* bg = find(at<void*>(screen, 0x1C), 8, 0x00CEF2D0);    // "bg_Group"
    void* title = find(bg, 0xA, 0x00CEC3D8);                    // "title_Text"
    host.gui_text_set_localised_source_00abaed0(title, host.image_global(0x00E08790), 1);
    {
        Cc13SweepBNativeString name;
        literal(name, 0xE, 0x00CEF2B0);  // "frameFlag_Icon"
        void* flag = host.widget_find_child_00aa7e00(bg, &name, 1);
        host.widget_vcall_88(flag, *static_cast<std::uint8_t*>(host.image_global(0x00E08798)) != 0 ? 1 : 0, 0, 1.0f);
        release_string(name, host);
    }
    void* logo = find(bg, 0x14, 0x00CEF298);  // "loadingLogo_FrameBox"

    const double four_fifteenths = f64_bits(0x3FD1111111111111ull);
    const float scale = static_cast<float>(at<float>(title, 0x114) / 960.0);
    if (scale > four_fifteenths) {
        const float height = host.gui_widget_get_size_00aa6740(logo)[1];
        const float pair[2] = {static_cast<float>(scale + four_fifteenths), height};
        host.widget_vcall_58_set_pair(logo, pair);
    } else {
        const float height = host.gui_widget_get_size_00aa6740(logo)[1];
        const float pair[2] = {0.53333336f, height};
        host.widget_vcall_58_set_pair(logo, pair);
    }

    void* game_now = host.global_00e188a8();
    if (at<std::int32_t>(game_now, 0x1FE4) != 0 && at<std::uint8_t>(game_now, 0x218C) == 0) {
        void* next = *static_cast<void**>(host.image_global(0x00E19698));
        at<std::uint8_t>(next, 4) = 1;
        at<std::uint8_t>(next, 5) = 1;
        host.frontend_screen_commit_visibility_004f83b0(next);
        host.screen_vcall_18_enter(next);
    }
}

// ---------------------------------------------------------------------------
// 007A8DA0 BSP_MovCam_StepDeckWalk
// __thiscall(ECX = camera), RET 0. EH frame 00C8BC68 (the stack list L,
// destroyed by 004BF8E0 on every path that reaches 007A917D).
// L = {count, head, tail} of 0Ch nodes {prev, next, unit} from operator new
// (a null allocation is still written through, as in the image). Source: the
// controlled player's (game+18CCh[game+18ECh]) +30h list +0DE4h when the
// player's +28h equals (camera+38Ch)+54h, else +0DD8h; its nodes {+4h next,
// +8h item}, item+4h the unit; units answering +5Ch(5) are appended.
// count < 1 (or an empty source) -> 00797070(camera, 0, 0) and done.
// camera+390h set: count < 2 -> 00797070(0, 0); else the index of +390h in L
// (by value, -1 when absent); index < count - 1 -> 00797070(00793380(&L,
// index + 1), 0), else 00797070(head value, 0).
// camera+390h clear: count 1 -> 00797070(head value, 0); otherwise every
// node is SCORED -- (camera+384h == 2 and ((camera+38Ch)+538h)+0BCh: the
// 0079F710 pose of that node (index camera+388h, NULL inside pointer), score
// = 0 - 42.0 * (float)sqrt(x*x + y*y) when the square is above 1e-10), then
// minus the 3D distance between camera+38Ch and the node's unit (both poses
// refreshed through 00414DB0 when byte +0C8h is clear), and the best score
// is tracked -- but NO index is kept: the pick after the loop walks `count`
// links from the head, which always lands on the LAST node. So the result
// is 00797070(last value, 0); the scoring only has the pose/sqrt side
// effects. Node walks stop at the last node; an empty walk yields 0.
// Then camera+390h set and +384h == 2: 0079F710(camera, out, +390h, +388h,
// &inside); inside clear and ((camera+38Ch)+538h)+0BCh set -> 007A8C60(camera,
// +390h) (cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60, which draws
// from the shared RNG 00BD2F10). 004BF8E0(&L). No RNG draw of its own.
void cc13_sweepB4_movcam_step_deck_walk_007a8da0(void* camera, Cc13SweepB4Host& host) {
    struct Node {
        Node* prev;
        Node* next;
        void* unit;
    };
    struct StackList {
        std::int32_t count;
        Node* head;
        Node* tail;
    } list = {0, nullptr, nullptr};

    // The inlined walk of k links from the head, stopping at the last node.
    auto value_at = [&](std::int32_t k) -> void* {
        Node* node = list.head;
        while (k > 0) {
            if (node == nullptr) {
                return nullptr;
            }
            if (node->next == nullptr) {
                break;
            }
            node = node->next;
            --k;
        }
        return node != nullptr ? node->unit : nullptr;
    };

    void* game = host.global_00e188a8();
    const std::int32_t player_index = at<std::int32_t>(game, 0x18EC);
    void* player = at<void*>(game, 0x18CC + static_cast<std::size_t>(player_index) * 4);
    const bool own = at<void*>(player, 0x28) == at<void*>(at<void*>(camera, 0x38C), 0x54);
    void* source = static_cast<std::uint8_t*>(at<void*>(player, 0x30)) + (own ? 0xDE4 : 0xDD8);
    for (void* node = at<void*>(source, 4); node != nullptr; node = at<void*>(node, 4)) {
        if (!host.object_vcall_5c_is_kind(at<void*>(at<void*>(node, 8), 4), 5)) {
            continue;
        }
        void* unit = at<void*>(at<void*>(node, 8), 4);
        auto* fresh = static_cast<Node*>(host.crt_operator_new_00bf681b(0xC));
        if (fresh != nullptr) {
            fresh->prev = nullptr;
            fresh->next = nullptr;
            fresh->unit = nullptr;
        }
        fresh->unit = unit;
        fresh->prev = list.tail;
        if (list.count != 0) {
            list.tail->next = fresh;
        } else {
            list.head = fresh;
        }
        list.count += 1;
        list.tail = fresh;
        fresh->next = nullptr;
    }

    if (list.count < 1) {
        host.movcam_set_look_target_00797070(camera, nullptr, 0);
        host.parent_list_clear_004bf8e0(&list);
        return;
    }
    void* current = at<void*>(camera, 0x390);
    if (current != nullptr) {
        if (list.count < 2) {
            host.movcam_set_look_target_00797070(camera, nullptr, 0);
        } else {
            std::int32_t index = 0;
            Node* node = list.head;
            while (node != nullptr && node->unit != current) {
                node = node->next;
                ++index;
            }
            if (node == nullptr) {
                index = -1;
            }
            if (index < list.count - 1) {
                host.movcam_set_look_target_00797070(camera, host.recon_list_payload_at_00793380(&list, index + 1),
                                                     0);
            } else {
                host.movcam_set_look_target_00797070(camera, list.head != nullptr ? list.head->unit : nullptr, 0);
            }
        }
    } else if (list.count == 1) {
        host.movcam_set_look_target_00797070(camera, list.head != nullptr ? list.head->unit : nullptr, 0);
    } else {
        float best = -1.0e10f;  // 00CE4ADC
        std::int32_t k = 0;
        for (; k < list.count; ++k) {
            float score = 0.0f;
            if (at<std::int32_t>(camera, 0x384) == 2 &&
                at<void*>(at<void*>(at<void*>(camera, 0x38C), 0x538), 0xBC) != nullptr) {
                float pose[2];
                host.movcam_vehicle_camera_pose_0079f710(camera, pose, value_at(k), at<std::int32_t>(camera, 0x388),
                                                         nullptr);
                const float sq = static_cast<float>(static_cast<double>(pose[1]) * pose[1] +
                                                    static_cast<double>(pose[0]) * pose[0]);
                float dist = 0.0f;
                if (sq > 1e-10) {
                    dist = static_cast<float>(host.crt_sqrt_00bf7030(sq));
                }
                score = static_cast<float>(0.0 - dist * 42.0);
            }
            void* unit = value_at(k);
            if (at<std::uint8_t>(unit, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(unit);
            }
            void* own_entity = at<void*>(camera, 0x38C);
            if (at<std::uint8_t>(own_entity, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(own_entity);
            }
            const float dx = at<float>(own_entity, 0xFC) - at<float>(unit, 0xFC);
            const float dy = at<float>(own_entity, 0x100) - at<float>(unit, 0x100);
            const float dz = at<float>(own_entity, 0x104) - at<float>(unit, 0x104);
            const float sq = static_cast<float>(
                (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) + static_cast<double>(dz) * dz);
            float dist = 0.0f;
            if (sq > 1e-10) {
                dist = static_cast<float>(host.crt_sqrt_00bf7030(sq));
            }
            score = static_cast<float>(static_cast<double>(score) - dist);
            if (score > best) {
                best = score;
            }
        }
        host.movcam_set_look_target_00797070(camera, value_at(k), 0);
    }

    void* target = at<void*>(camera, 0x390);
    if (target != nullptr && at<std::int32_t>(camera, 0x384) == 2) {
        float pose[2];
        std::uint8_t inside = 0;
        host.movcam_vehicle_camera_pose_0079f710(camera, pose, target, at<std::int32_t>(camera, 0x388), &inside);
        if (inside == 0 && at<void*>(at<void*>(at<void*>(camera, 0x38C), 0x538), 0xBC) != nullptr) {
            cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60(camera, at<void*>(camera, 0x390), host);
        }
    }
    host.parent_list_clear_004bf8e0(&list);
}

// ---------------------------------------------------------------------------
// 005A7550 BSP_HudMapArrowWidget_SetWaypointPath
// __thiscall(ECX = arrow; map, const float* start, path, int first_index,
// int curve_arg, int texture_index), RET 18h. EH frame 00C719B0 (the local
// vector and the texture-name temporary).
// arrow+4h and the eight curves arrow+8h..+24h hidden (+34h(0)). offset_x =
// byte game+61Fh ? 1000.0f : 0.0f. A local 12-byte vector V {first, last,
// end} gets start, then path[i] for i = first_index while i < 0059CAC0(path)
// (re-called each pass; path {+4h first, +8h last}, the index checked through
// 00BF6713). For segment s = 0.. while s < size(V) - 1 (unsigned; both
// endpoints index-checked), curve = arrow+8h[s]: s > 0 and 0059C9B0(game,
// V[s], 10.0f) -> skipped; 0059C9B0(game, V[s+1], 10.0f) -> skipped;
// otherwise 00ABC920(curve) then for p = V[s], V[s+1]: u = (float)((p.x -
// map+54Ch) / map+30h + [00E08890]), v = (float)((map+550h - p.z) / map+34h
// + [00E08894]); 00ABC970(curve, {(float)(u + offset_x), (float)(v + 0.0)},
// (float)(map+5CCh * 0.014 (double 00CF0110))); curve->+50h(curve_arg); a
// native string of the C string [00E08884 + texture_index*4] to 00ABDD80 and
// released; 00ABCA10(curve); curve->+34h(1). Curves s..7 not reached are
// hidden; V's block is freed (00BF65AC). The skipped curves stay hidden.
void cc13_sweepB4_hud_map_arrow_set_waypoint_path_005a7550(void* arrow, void* map, const float* start,
                                                           void* path, std::int32_t first_index,
                                                           std::int32_t curve_arg, std::int32_t texture_index,
                                                           Cc13SweepB4Host& host) {
    struct Vec12 {
        float* first;
        float* last;
        float* end;
    } points = {nullptr, nullptr, nullptr};
    auto size_of = [&]() -> std::uint32_t {
        return points.first == nullptr ? 0u : static_cast<std::uint32_t>((points.last - points.first) / 3);
    };

    host.widget_vcall_34_set_visible(at<void*>(arrow, 4), 0);
    for (int k = 0; k < 8; ++k) {
        host.widget_vcall_34_set_visible(at<void*>(arrow, 8 + static_cast<std::size_t>(k) * 4), 0);
    }
    const float offset_x = at<std::uint8_t>(host.global_00e188a8(), 0x61F) != 0 ? 1000.0f : 0.0f;
    host.vector12_push_back_004215d0(&points, start);
    for (std::int32_t i = first_index; i < host.vector12_size_0059cac0(path); ++i) {
        const float* path_first = at<float*>(path, 4);
        if (path_first == nullptr ||
            !(static_cast<std::uint32_t>(i) <
              static_cast<std::uint32_t>((at<float*>(path, 8) - path_first) / 3))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        const float* p = at<float*>(path, 4) + static_cast<std::size_t>(i) * 3;
        const float copy[3] = {p[0], p[1], p[2]};
        host.vector12_push_back_004215d0(&points, copy);
    }

    const double width_scale = f64_bits(0x3F8CAC0840000000ull);  // 0.014 (00CF0110)
    auto map_point = [&](const float* p, float* out) {
        const float half_v = *static_cast<float*>(host.image_global(0x00E08894));
        const float half_u = *static_cast<float*>(host.image_global(0x00E08890));
        const float v = static_cast<float>(
            (static_cast<double>(at<float>(map, 0x550)) - p[2]) / at<float>(map, 0x34) + half_v);
        const float u = static_cast<float>(
            (static_cast<double>(p[0]) - at<float>(map, 0x54C)) / at<float>(map, 0x30) + half_u);
        out[0] = static_cast<float>(static_cast<double>(u) + offset_x);
        out[1] = static_cast<float>(static_cast<double>(v) + 0.0);
    };

    std::uint32_t segment = 0;
    while (segment < size_of() - 1u) {
        if (points.first == nullptr || !(segment < size_of())) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        const float* p0 = points.first + static_cast<std::size_t>(segment) * 3;
        if (points.first == nullptr || !(segment + 1 < size_of())) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        const float* p1 = points.first + static_cast<std::size_t>(segment) * 3 + 3;
        void* curve = at<void*>(arrow, 8 + static_cast<std::size_t>(segment) * 4);
        const bool skip =
            (static_cast<std::int32_t>(segment) > 0 &&
             host.world_point_beyond_map_margin_0059c9b0(host.global_00e188a8(), p0, 10.0f)) ||
            host.world_point_beyond_map_margin_0059c9b0(host.global_00e188a8(), p1, 10.0f);
        if (!skip) {
            host.gui_curve_clear_points_00abc920(curve);
            float a[2];
            map_point(p0, a);
            host.gui_curve_add_point_00abc970(
                curve, a, static_cast<float>(at<float>(map, 0x5CC) * width_scale));
            float b[2];
            map_point(p1, b);
            host.gui_curve_add_point_00abc970(
                curve, b, static_cast<float>(at<float>(map, 0x5CC) * width_scale));
            host.widget_vcall_50(curve, curve_arg);
            const char* texture =
                *static_cast<const char**>(host.image_global(0x00E08884 + static_cast<std::uint32_t>(texture_index) * 4));
            Cc13SweepBNativeString name = {0, nullptr};
            host.native_string_resize_0041dd40(&name, static_cast<std::uint32_t>(std::strlen(texture)), 1);
            if (name.data != nullptr) {
                host.crt_memcpy_00bf7680(name.data, texture, name.length + 1);
            }
            host.gui_curve_set_texture_00abdd80(curve, &name);
            release_string(name, host);
            host.gui_curve_rebuild_00abca10(curve);
            host.widget_vcall_34_set_visible(curve, 1);
        }
        ++segment;
    }
    for (std::uint32_t k = segment; static_cast<std::int32_t>(k) < 8; ++k) {
        host.widget_vcall_34_set_visible(at<void*>(arrow, 8 + static_cast<std::size_t>(k) * 4), 0);
    }
    if (points.first != nullptr) {
        host.crt_operator_delete_00bf65ac(points.first);
    }
}

// ---------------------------------------------------------------------------
// 0053EE00 BSP_FreeCameraEffect_Update
// __thiscall(ECX = effect; float dt, const camera block*), RET 8. EH frame
// 00C6C9A3 (the stack entry and the by-value smart pointer).
// effect: +0h text widget, +4h checked vector of 30h-byte records {proxy,
// first +8h, last +0Ch}, +14h list (+18h head), +20h index, +24h repeat timer,
// +28h caption timer. camera: +10h..+18h and +30h..+38h points, +20h..+28h a
// direction.
// +28h > 0: 00ABAED0(+0h, &record[+20h]+1Ch, 1) (index checked), +0h->+34h(1),
// +28h -= dt; else +0h->+34h(0). Action 65h pressed (004C43C0): +28h > 0 ->
// +20h = (+20h + 1) % count (unsigned; a zero count faults as in the image);
// +28h = 1.0f. fire = byte record+18h ? action 64h pressed : (input+4h)+12C0h
// has byte +28h set and float +24h > 0. +24h > 0 -> +24h -= dt. Only when
// fire and +24h <= 0 (COMISS + JB, NaN stops): +24h = record+8h; a stack
// entry E (0053DC70) gets d = camera+30h - camera+10h at +4h and +10h,
// camera+20h * record+0h at +1Ch, record+0Ch at +28h, the index at +30h;
// record+24h (the effect definition) non-null -> m = {1,0,0,0, 0,1,0,0,
// camera+20h..+28h,0, 0,0,0,1}, 0085DC80(m), m[12..14] = d, 00868420(&out,
// [00E188A8]+19ECh, definition (+1 reference), m, 0, 0), 004845D0(&E+2Ch,
// result), out released (InterlockedDecrement, vtable +0h at zero), and E+2Ch
// non-null -> 00867790(E+2Ch, 0.0f, [00E188A8]+19FCh). E is then inserted
// before the +14h list head (0053E890, 0053E930, link stores) and destroyed
// (0053DCE0). Each record access after the first goes through 0053D300.
void cc13_sweepB4_free_camera_effect_update_0053ee00(void* effect, float dt, const void* camera,
                                                     Cc13SweepB4Host& host) {
    void* const cam = const_cast<void*>(camera);
    void* const records = static_cast<std::uint8_t*>(effect) + 4;
    auto record_count = [&]() -> std::uint32_t {
        std::uint8_t* first = at<std::uint8_t*>(effect, 8);
        return first == nullptr ? 0u : static_cast<std::uint32_t>((at<std::uint8_t*>(effect, 0xC) - first) / 0x30);
    };
    auto checked_record = [&]() -> std::uint8_t* {
        const std::uint32_t index = at<std::uint32_t>(effect, 0x20);
        if (at<std::uint8_t*>(effect, 8) == nullptr || !(index < record_count())) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        return at<std::uint8_t*>(effect, 8) + static_cast<std::size_t>(index) * 0x30;
    };

    if (at<float>(effect, 0x28) > 0.0f) {
        std::uint8_t* record = checked_record();
        host.gui_text_set_localised_source_00abaed0(at<void*>(effect, 0), record + 0x1C, 1);
        host.widget_vcall_34_set_visible(at<void*>(effect, 0), 1);
        at<float>(effect, 0x28) = static_cast<float>(static_cast<double>(at<float>(effect, 0x28)) - dt);
    } else {
        host.widget_vcall_34_set_visible(at<void*>(effect, 0), 0);
    }
    if (host.input_action_was_pressed_004c43c0(host.global_00e188a8(), 0x65)) {
        if (at<float>(effect, 0x28) > 0.0f) {
            at<std::uint32_t>(effect, 0x20) = (at<std::uint32_t>(effect, 0x20) + 1u) % record_count();
        }
        at<float>(effect, 0x28) = 1.0f;
    }
    bool fire;
    if (checked_record()[0x18] != 0) {
        fire = host.input_action_was_pressed_004c43c0(host.global_00e188a8(), 0x64);
    } else {
        std::uint8_t* block = at<std::uint8_t*>(host.input_manager_singleton_004bec00(), 4) + 0x12C0;
        fire = at<std::uint8_t>(block, 0x28) != 0 && at<float>(block, 0x24) > 0.0f;
    }
    const float repeat = at<float>(effect, 0x24);
    if (repeat > 0.0f) {
        at<float>(effect, 0x24) = static_cast<float>(static_cast<double>(repeat) - dt);
    }
    if (!fire || !(at<float>(effect, 0x24) <= 0.0f)) {
        return;
    }

    at<float>(effect, 0x24) =
        at<float>(host.free_camera_effect_record_at_0053d300(records, at<std::int32_t>(effect, 0x20)), 8);
    alignas(8) std::uint8_t entry[0x40] = {};
    host.free_camera_effect_entry_construct_0053dc70(entry);
    const std::int32_t index = at<std::int32_t>(effect, 0x20);
    const float dx = at<float>(cam, 0x30) - at<float>(cam, 0x10);
    const float dy = at<float>(cam, 0x34) - at<float>(cam, 0x14);
    const float dz = at<float>(cam, 0x38) - at<float>(cam, 0x18);
    at<std::int32_t>(entry, 0x30) = index;
    at<float>(entry, 0x10) = dx;
    at<float>(entry, 0x14) = dy;
    at<float>(entry, 0x18) = dz;
    at<float>(entry, 0x04) = dx;
    at<float>(entry, 0x08) = dy;
    at<float>(entry, 0x0C) = dz;
    const float scale = at<float>(host.free_camera_effect_record_at_0053d300(records, at<std::int32_t>(effect, 0x20)), 0);
    at<float>(entry, 0x1C) = static_cast<float>(static_cast<double>(at<float>(cam, 0x20)) * scale);
    at<float>(entry, 0x20) = static_cast<float>(static_cast<double>(at<float>(cam, 0x24)) * scale);
    at<float>(entry, 0x24) = static_cast<float>(static_cast<double>(at<float>(cam, 0x28)) * scale);
    at<float>(entry, 0x28) =
        at<float>(host.free_camera_effect_record_at_0053d300(records, at<std::int32_t>(effect, 0x20)), 0xC);
    void* record4 = host.free_camera_effect_record_at_0053d300(records, at<std::int32_t>(effect, 0x20));
    if (at<void*>(record4, 0x24) != nullptr) {
        float m[16] = {1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
                       at<float>(cam, 0x20), at<float>(cam, 0x24), at<float>(cam, 0x28), 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f};
        host.matrix_orthonormalize_rows_0085dc80(m);
        m[12] = at<float>(entry, 0x10);
        m[13] = at<float>(entry, 0x14);
        m[14] = at<float>(entry, 0x18);
        void* system = at<void*>(host.global_00e188a8(), 0x19EC);
        void* record5 = host.free_camera_effect_record_at_0053d300(records, at<std::int32_t>(effect, 0x20));
        void* definition = at<void*>(record5, 0x24);
        if (definition != nullptr) {
            host.interlocked_increment_00ce221c(&at<std::int32_t>(definition, 4));
        }
        void* out = nullptr;
        void* result = host.point_effect_create_from_matrix_00868420(&out, system, definition, m, 0, 0);
        host.retained_reference_assign_004845d0(entry + 0x2C, result);
        if (out != nullptr) {
            void* held = out;
            if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(held, 4)) == 0) {
                host.refcounted_vcall_00_destroy(held);
            }
            out = nullptr;
        }
        void* point_effect = at<void*>(entry, 0x2C);
        if (point_effect != nullptr) {
            host.effect_update_children_00867790(point_effect, 0.0f, at<void*>(host.global_00e188a8(), 0x19FC));
        }
    }
    void* list = static_cast<std::uint8_t*>(effect) + 0x14;
    void* head = at<void*>(effect, 0x18);
    void* node = host.free_camera_effect_list_buy_node_0053e890(list, head, at<void*>(head, 4), entry);
    host.free_camera_effect_list_grow_0053e930(list, 1);
    at<void*>(head, 4) = node;
    at<void*>(at<void*>(node, 4), 0) = node;
    host.free_camera_effect_entry_destruct_0053dce0(entry);
}

// ---------------------------------------------------------------------------
// 005E9BE0 BSP_MultiMainMenuScreen_UpdateSessionList (ledger name; it moves
// the award-grid cursor)
// __thiscall(ECX = screen), RET 0. EH frame 00C75478 (the award record copy).
// The grid list L = (screen+0D8h)+64h {+4h head}, nodes {next, prev, widget};
// every dereference/advance is checked against the head and the container
// identity (00BF6713).
// From the first node, skip to the first whose widget+76h byte is set; then
// advance (+0E4h * 3 + +0E0h) nodes while not at the end. At the end: take
// the last node (prev of the head) and recompute the cursor from its local
// position against +0DCh: col = _ftol2(round(q)) with q = (float)((x - x0) /
// 0.084375 (double 00CF29A0)), r = (float)floor(q), +1.0 when |(float)(q - r)|
// > 0.5; the same for the row with y and 0.102778 (double 00CF2998); +0E0h =
// col, +0E4h = row. Then y = 00AA6750(widget)[1]: d = (float)(y - +0ECh), e1
// = (float)(d - c), e2 = (float)((d - c) - c) with c = 0.102778; +0F0h = d
// when |e1| > |d|, else e1 when |e2| > |e1|, else e2 (FCOMIP + JA/JBE, |x| by
// -0.0 - x on the non-positive side). R = copy of 0050FC30([00E19900]+10h,
// widget->+28h()); +0E8h->+48h({1, 1}); widget->+48h({1.25, 1.25});
// 00AA7DC0(+0D4h, 00AA67F0(widget)); 00AA7910(+0D4h, -100.0f); +0D4h shown;
// 00ABAED0(+48h, &R, 1); 00ABAED0(+4Ch, &R+0Ch, 1); +0E8h = widget; R
// destroyed (00501FA0).
void cc13_sweepB4_multi_main_update_session_list_005e9be0(void* screen, Cc13SweepB4Host& host) {
    auto check = [&](bool ok) {
        if (!ok) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    std::uint8_t* const container = at<std::uint8_t*>(screen, 0xD8) + 0x64;
    auto end_node = [&]() -> void* {
        std::uint8_t* holder = at<std::uint8_t*>(screen, 0xD8);
        void* end = at<void*>(holder, 0x68);
        check(container == holder + 0x64);
        return end;
    };
    auto head = [&]() { return at<void*>(container, 4); };
    void* node = at<void*>(at<void*>(at<void*>(screen, 0xD8), 0x68), 0);

    while (node != end_node()) {
        check(node != head());
        if (at<std::uint8_t>(at<void*>(node, 8), 0x76) != 0) {
            break;
        }
        check(node != head());
        node = at<void*>(node, 0);
    }
    std::int32_t steps = at<std::int32_t>(screen, 0xE4) * 3 + at<std::int32_t>(screen, 0xE0);
    while (node != end_node() && steps > 0) {
        --steps;
        check(node != head());
        node = at<void*>(node, 0);
    }
    if (node == end_node()) {
        node = at<void*>(node, 4);
        if (node == head()) {
            host.crt_invalid_parameter_noinfo_00bf6713();
            check(node != head());
        }
        auto round_cell = [&](float position, float origin, double pitch) -> std::int32_t {
            const float q = static_cast<float>((static_cast<double>(position) - origin) / pitch);
            const float r = static_cast<float>(host.crt_floor_00bf85b0(q));
            const float diff = static_cast<float>(static_cast<double>(q) - r);
            double value = r;
            if (std::fabs(diff) > 0.5f) {
                value = value + 1.0;
            }
            return ftol_trunc(static_cast<float>(value));
        };
        const float x = host.gui_widget_get_local_position_00aa67f0(at<void*>(node, 8))[0];
        const float x0 = host.gui_widget_get_local_position_00aa67f0(at<void*>(screen, 0xDC))[0];
        const std::int32_t col = round_cell(x, x0, f64_bits(0x3FB59999A0000000ull));
        check(node != head());
        const float y = host.gui_widget_get_local_position_00aa67f0(at<void*>(node, 8))[1];
        const float y0 = host.gui_widget_get_local_position_00aa67f0(at<void*>(screen, 0xDC))[1];
        const std::int32_t row = round_cell(y, y0, f64_bits(0x3FBA4FA8C0000000ull));
        at<std::int32_t>(screen, 0xE0) = col;
        at<std::int32_t>(screen, 0xE4) = row;
    }

    check(node != head());
    float resolved[3];
    const float y = host.widget_get_resolved_position_00aa6750(at<void*>(node, 8), resolved)[1];
    const double pitch = f64_bits(0x3FBA4FA8C0000000ull);  // 0.102778 (00CF2998)
    const float d = static_cast<float>(static_cast<double>(y) - at<float>(screen, 0xEC));
    const double d_minus = static_cast<double>(d) - pitch;
    const float e1 = static_cast<float>(d_minus);
    const float e2 = static_cast<float>(d_minus - pitch);
    auto magnitude = [](float v) { return v > 0.0f ? v : -0.0f - v; };
    if (magnitude(e1) > magnitude(d)) {
        at<float>(screen, 0xF0) = d;
    } else if (magnitude(e2) > magnitude(e1)) {
        at<float>(screen, 0xF0) = e1;
    } else {
        at<float>(screen, 0xF0) = e2;
    }

    check(node != head());
    void* name = host.widget_vcall_28_name(at<void*>(node, 8));
    void* found = host.award_table_find_record_0050fc30(
        static_cast<std::uint8_t*>(*static_cast<void**>(host.image_global(0x00E19900))) + 0x10, name);
    alignas(8) std::uint8_t record[0x54];
    host.award_record_copy_construct_0050a210(record, found);
    const float normal[2] = {1.0f, 1.0f};
    host.widget_vcall_48_set_pair(at<void*>(screen, 0xE8), normal);
    check(node != head());
    const float grown[2] = {1.25f, 1.25f};
    host.widget_vcall_48_set_pair(at<void*>(node, 8), grown);
    check(node != head());
    host.gui_widget_set_local_position_00aa7dc0(at<void*>(screen, 0xD4),
                                                host.gui_widget_get_local_position_00aa67f0(at<void*>(node, 8)));
    host.widget_set_local_z_00aa7910(at<void*>(screen, 0xD4), -100.0f);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xD4), 1);
    host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x48), record, 1);
    host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x4C), record + 0xC, 1);
    check(node != head());
    at<void*>(screen, 0xE8) = at<void*>(node, 8);
    host.award_record_destroy_00501fa0(record);
}

// ---------------------------------------------------------------------------
// 00570300 BSP_MultiSessionBrowserScreen_PrepareEnter
// __thiscall(ECX = screen), RET 0. EBP frame, EH frame 00C6EEA0 (the name
// temporaries, each built inline: {0,0}, 0041DD40(length, 1), memcpy of
// length + 1 when data is non-null; released right after use).
// For row r = 0..8 of the list box +34h: 00A9D750(+34h, 00AAB4C0(+0ECh, 0),
// 0, 0); label = 00AA7E00(00A9BE00(+34h, r), "setting_name_Text", 1);
// 00ABAED0(label, the C string [00E086C4 + 4r] as a native string, 1); byte
// label+1F0h = 0; label->+58h({0.484375f, 00AA6740(label)[1]}); label+30h =
// -0.013541667f (00CEF0E4), label+34h = 0.0f; "scroll_right_Icon" gets
// 00AA6BC0(screen+0Ch, 0) and, looked up again, +0D8h = r; the same for
// "scroll_left_Icon". Then 00A9CEB0(+34h, 0.484375f, 0); (+34h)+12Ch =
// {0.0f, 0.0027777778f (00CEE600), 0.0f}; 00A9AC40(+34h, screen+8);
// (+34h)+128h = 0.008333334f (00CEF0B8); bytes +0F0h = 1, +0F1h = 0;
// 0056D660(screen) (cc13_sweepB3_session_browser_fill_settings_0056d660);
// +0ECh->+34h(0).
void cc13_sweepB4_session_browser_prepare_enter_00570300(void* screen, Cc13SweepB4Host& host) {
    auto named_child = [&](std::int32_t row, const char* text, std::uint32_t length) {
        Cc13SweepBNativeString name = {0, nullptr};
        host.native_string_resize_0041dd40(&name, length, 1);
        if (name.data != nullptr) {
            host.crt_memcpy_00bf7680(name.data, text, name.length + 1);
        }
        void* child = host.widget_find_child_00aa7e00(host.gui_listbox_row_00a9be00(at<void*>(screen, 0x34), row),
                                                      &name, 1);
        return std::make_pair(child, name);
    };
    const char* setting_name = static_cast<const char*>(host.image_global(0x00CEF0EC));
    const char* scroll_right = static_cast<const char*>(host.image_global(0x00CEF0D0));
    const char* scroll_left = static_cast<const char*>(host.image_global(0x00CEF0BC));
    void* const listener = static_cast<std::uint8_t*>(screen) + 0xC;

    for (std::int32_t row = 0; row < 9; ++row) {
        void* clone = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xEC), nullptr);
        host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x34), clone, nullptr, 0);

        auto label = named_child(row, setting_name, 0x11);
        release_string(label.second, host);
        const char* caption = *static_cast<const char**>(host.image_global(0x00E086C4 + static_cast<std::uint32_t>(row) * 4));
        Cc13SweepBNativeString text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(caption)), 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, caption, text.length + 1);
        }
        host.gui_text_set_localised_source_00abaed0(label.first, &text, 1);
        release_string(text, host);
        at<std::uint8_t>(label.first, 0x1F0) = 0;
        const float height = host.gui_widget_get_size_00aa6740(label.first)[1];
        const float pair[2] = {0.484375f, height};
        host.widget_vcall_58_set_pair(label.first, pair);
        at<std::uint32_t>(label.first, 0x30) = 0xBC5DDDDEu;  // -0.013541667f (00CEF0E4)
        at<float>(label.first, 0x34) = 0.0f;

        auto right = named_child(row, scroll_right, 0x11);
        host.gui_widget_set_listener_00aa6bc0(right.first, listener, 0);
        release_string(right.second, host);
        auto right_again = named_child(row, scroll_right, 0x11);
        at<std::int32_t>(right_again.first, 0xD8) = row;
        release_string(right_again.second, host);
        auto left = named_child(row, scroll_left, 0x10);
        host.gui_widget_set_listener_00aa6bc0(left.first, listener, 0);
        release_string(left.second, host);
        auto left_again = named_child(row, scroll_left, 0x10);
        at<std::int32_t>(left_again.first, 0xD8) = row;
        release_string(left_again.second, host);
    }
    void* listbox = at<void*>(screen, 0x34);
    host.gui_listbox_set_field_124_00a9ceb0(listbox, 0.484375f, 0);
    at<float>(at<void*>(screen, 0x34), 0x12C) = 0.0f;
    at<std::uint32_t>(at<void*>(screen, 0x34), 0x130) = 0x3B360B61u;  // 0.0027777778f (00CEE600)
    at<float>(at<void*>(screen, 0x34), 0x134) = 0.0f;
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x34), static_cast<std::uint8_t*>(screen) + 8);
    at<std::uint32_t>(at<void*>(screen, 0x34), 0x128) = 0x3C088889u;  // 0.008333334f (00CEF0B8)
    at<std::uint8_t>(screen, 0xF0) = 1;
    at<std::uint8_t>(screen, 0xF1) = 0;
    cc13_sweepB3_session_browser_fill_settings_0056d660(screen, host);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xEC), 0);
}

// ---------------------------------------------------------------------------
// 005EBDD0 BSP_MultiModeSelectorScreen_RefreshServerInfo
// __thiscall(ECX = screen), RET 0. EH frame 00C75698.
// session = [00E188A8] + 1FF0h. screen+10h non-null with n = (+10h)+18h
// non-zero (unsigned) -> session+6Ch = (float)n (FILD + 2^32 fix-up).
// w = 00AA7E00(+0A4h, "server_name_t_Text", 1) (name built inline and
// released); S = 0041E870(session as a C string) (alive to the end); 00ABAED0(w,
// &S, S.data && _stricmp(S.data, "globals.newplayer") == 0 ? 1 : 0).
// 00AA7E00(+0A4h, "connection_t_Text", 1)->+34h(0). 00578320(+0ACh,
// CVTTSS2SI session+6Ch). 00ABAED0(+0C0h, 004260B0(+0E4h), 1) and (+0C4h,
// 004260B0(+0E8h), 1), each released. byte [00F8A2FC]+4Dh set: +0C4h/+0D0h
// ->+7Ch(1) and colour {0.4705882 x3, 1}; else ->+7Ch(0) and {0.894118,
// 0.8, 0.596078, 1} (00CF2C90/00CE74F8/00CF2C8C) through vtable +50h.
// +0F0h < 0 -> 0. 00ABB000(+0C8h, C string [00E08E00 + 4*+0F0h], +0DCh, 1)
// and (+0CCh, [00E08E00 + 4*+0ECh], +0DCh, 1) with the strings built by
// 0041E870 and released; +0D4h->+88h(word +0F0h, 0, 1.0f), +0D8h->
// +88h(word +0ECh, 0, 1.0f); S released.
void cc13_sweepB4_mode_selector_refresh_server_info_005ebdd0(void* screen, Cc13SweepB4Host& host) {
    auto* session = static_cast<std::uint8_t*>(host.global_00e188a8()) + 0x1FF0;
    void* info = at<void*>(screen, 0x10);
    if (info != nullptr && at<std::uint32_t>(info, 0x18) != 0) {
        at<float>(session, 0x6C) = static_cast<float>(at<std::uint32_t>(info, 0x18));
    }
    auto find = [&](const char* text, std::uint32_t length) {
        Cc13SweepBNativeString name = {0, nullptr};
        host.native_string_resize_0041dd40(&name, length, 1);
        if (name.data != nullptr) {
            host.crt_memcpy_00bf7680(name.data, text, name.length + 1);
        }
        void* child = host.widget_find_child_00aa7e00(at<void*>(screen, 0xA4), &name, 1);
        release_string(name, host);
        return child;
    };
    void* server_name = find(static_cast<const char*>(host.image_global(0x00CF2CAC)), 0x12);
    Cc13SweepBNativeString session_name;
    host.native_string_assign_0041e870(&session_name, reinterpret_cast<const char*>(session));
    bool localise = false;
    if (session_name.data != nullptr) {
        localise = host.crt_stricmp_00bf7fbf(static_cast<const char*>(session_name.data),
                                             static_cast<const char*>(host.image_global(0x00CEF794))) == 0;
    }
    host.gui_text_set_localised_source_00abaed0(server_name, &session_name, localise ? 1 : 0);
    void* connection = find(static_cast<const char*>(host.image_global(0x00CF2C98)), 0x11);
    host.widget_vcall_34_set_visible(connection, 0);
    host.session_row_set_connection_icons_00578320(at<void*>(screen, 0xAC),
                                                   ftol_trunc(at<float>(session, 0x6C)));  // CVTTSS2SI
    {
        Cc13SweepBNativeString number;
        host.native_string_from_int_004260b0(&number, at<std::int32_t>(screen, 0xE4));
        host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0xC0), &number, 1);
        release_string(number, host);
    }
    {
        Cc13SweepBNativeString number;
        host.native_string_from_int_004260b0(&number, at<std::int32_t>(screen, 0xE8));
        host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0xC4), &number, 1);
        release_string(number, host);
    }
    void* online = *static_cast<void**>(host.image_global(0x00F8A2FC));
    if (at<std::uint8_t>(online, 0x4D) != 0) {
        host.widget_vcall_7c(at<void*>(screen, 0xC4), 1);
        host.widget_vcall_7c(at<void*>(screen, 0xD0), 1);
        const float grey[4] = {0.47058821f, 0.47058821f, 0.47058821f, 1.0f};  // 00CF2C94
        host.widget_vcall_50_colour(at<void*>(screen, 0xC4), grey);
        const float grey2[4] = {0.47058821f, 0.47058821f, 0.47058821f, 1.0f};
        host.widget_vcall_50_colour(at<void*>(screen, 0xD0), grey2);
    } else {
        host.widget_vcall_7c(at<void*>(screen, 0xC4), 0);
        host.widget_vcall_7c(at<void*>(screen, 0xD0), 0);
        const float sand[4] = {0.89411801f, 0.80000001f, 0.59607798f, 1.0f};
        host.widget_vcall_50_colour(at<void*>(screen, 0xC4), sand);
        const float sand2[4] = {0.89411801f, 0.80000001f, 0.59607798f, 1.0f};
        host.widget_vcall_50_colour(at<void*>(screen, 0xD0), sand2);
    }
    if (at<std::int32_t>(screen, 0xF0) < 0) {
        at<std::int32_t>(screen, 0xF0) = 0;
    }
    auto ellipsized = [&](std::size_t widget_offset, std::int32_t index) {
        Cc13SweepBNativeString text;
        host.native_string_assign_0041e870(
            &text, *static_cast<const char**>(host.image_global(0x00E08E00 + static_cast<std::uint32_t>(index) * 4)));
        host.gui_text_set_ellipsized_source_00abb000(at<void*>(screen, widget_offset), &text, at<float>(screen, 0xDC), 1);
        release_string(text, host);
    };
    ellipsized(0xC8, at<std::int32_t>(screen, 0xF0));
    ellipsized(0xCC, at<std::int32_t>(screen, 0xEC));
    host.widget_vcall_88(at<void*>(screen, 0xD4), at<std::uint16_t>(screen, 0xF0), 0, 1.0f);
    host.widget_vcall_88(at<void*>(screen, 0xD8), at<std::uint16_t>(screen, 0xEC), 0, 1.0f);
    release_string(session_name, host);
}

// ---------------------------------------------------------------------------
// 00642C20 BSP_InGameHudMarkers_AddCommandUnitMarker
// __thiscall(ECX = markers screen; unit), RET 4. EH frame 00C7A450 (the text
// T and the concatenation temporaries).
// 00414DB0 when byte unit+0C8h is clear; P = unit+0FCh..+104h; kind 8 and
// 00852860(unit): the controlled entity [00E188A8]+1ED4h (pose refreshed when
// needed) above y 0 (COMISS + JBE) -> P.y = 0.0f. 00638E50(&P, &Q) false ->
// return. 0063A6C0(screen, P, unit, &collapsed, &min_x, &min_y, &max_x,
// &max_y). player = game+18CCh[unit+1ACh]; T = "[" + player+78h + "] " +
// player+58h (0041E870 pieces, 004261A0 three times, inline assign into T,
// the pieces released newest first) when the C string player+78h is
// non-empty, else 0041E350(&T, player+58h). Colour: game+18ECh outside 0..7
// -> the function-local static {1,1,1,1} (guard 00E197B8 bit 0, storage
// 00E197A8..B4), else 00432650() + (player+28h ? 128h : 108h), four dwords.
// marker = 00640940(screen); 00AA7D00(marker+4h, Q.x, min_y);
// (marker+4h)->+50h(&colour); 00ABAED0(marker+4h, &T, 0); +34h(1); T
// released.
void cc13_sweepB4_hud_markers_add_command_unit_marker_00642c20(void* screen, void* unit, Cc13SweepB4Host& host) {
    if (at<std::uint8_t>(unit, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
    }
    float position[3] = {at<float>(unit, 0xFC), at<float>(unit, 0x100), at<float>(unit, 0x104)};
    if (host.object_vcall_5c_is_kind(unit, 8) && host.entity_is_below_altitude_third_00852860(unit)) {
        void* controlled = at<void*>(host.global_00e188a8(), 0x1ED4);
        if (at<std::uint8_t>(controlled, 0xC8) == 0) {
            host.entity_pose_refresh_world_00414db0(controlled);
        }
        if (at<float>(controlled, 0x100) > 0.0f) {
            position[1] = 0.0f;
        }
    }
    float projected[3];
    if (!host.hud_marker_project_and_clip_00638e50(position, projected)) {
        return;
    }
    std::uint8_t collapsed = 0;
    float min_x = 0.0f;
    float min_y = 0.0f;
    float max_x = 0.0f;
    float max_y = 0.0f;
    host.hud_marker_project_unit_bounds_0063a6c0(screen, position[0], position[1], position[2], unit, &collapsed,
                                                 &min_x, &min_y, &max_x, &max_y);
    const float label_y = min_y;
    void* game = host.global_00e188a8();
    auto* player = static_cast<std::uint8_t*>(
        at<void*>(game, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(unit, 0x1AC)) * 4));

    Cc13SweepBNativeString text = {0, nullptr};
    const char* clan = reinterpret_cast<const char*>(player + 0x78);
    if (std::strlen(clan) != 0) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, reinterpret_cast<const char*>(player + 0x58));
        Cc13SweepBNativeString close;
        host.native_string_assign_0041e870(&close, static_cast<const char*>(host.image_global(0x00CF5984)));
        Cc13SweepBNativeString tag;
        host.native_string_assign_0041e870(&tag, clan);
        Cc13SweepBNativeString open;
        host.native_string_assign_0041e870(&open, static_cast<const char*>(host.image_global(0x00CF5980)));
        Cc13SweepBNativeString first;
        host.native_string_concat_004261a0(&open, &first, &tag);
        Cc13SweepBNativeString second;
        host.native_string_concat_004261a0(&first, &second, &close);
        Cc13SweepBNativeString whole;
        host.native_string_concat_004261a0(&second, &whole, &name);
        if (&text != &whole) {
            host.native_string_resize_0041dd40(&text, whole.length, 1);
            if (whole.length != 0) {
                host.crt_memcpy_00bf7680(text.data, whole.data, text.length);
            }
        }
        release_string(whole, host);
        release_string(second, host);
        release_string(first, host);
        release_string(open, host);
        release_string(tag, host);
        release_string(close, host);
        release_string(name, host);
    } else {
        host.native_string_assign_cstring_0041e350(&text, reinterpret_cast<const char*>(player + 0x58));
    }

    std::uint32_t colour[4];
    const std::int32_t controlled_index = at<std::int32_t>(host.global_00e188a8(), 0x18EC);
    if (controlled_index < 0 || controlled_index > 7) {
        std::uint32_t& guard = *static_cast<std::uint32_t*>(host.image_global(0x00E197B8));
        auto* fallback = static_cast<float*>(host.image_global(0x00E197A8));
        if ((guard & 1u) == 0) {
            guard |= 1u;
            fallback[0] = 1.0f;
            fallback[1] = 1.0f;
            fallback[2] = 1.0f;
            fallback[3] = 1.0f;
        }
        std::memcpy(colour, fallback, sizeof colour);
    } else {
        auto* config = static_cast<std::uint8_t*>(host.global_config_00432650());
        std::memcpy(colour, config + (at<std::int32_t>(player, 0x28) == 0 ? 0x108 : 0x128), sizeof colour);
    }

    const float label_x = projected[0];
    void* marker = host.hud_markers_acquire_command_unit_marker_00640940(screen);
    host.widget_set_local_xy_00aa7d00(at<void*>(marker, 4), label_x, label_y);
    host.widget_vcall_50_colour(at<void*>(marker, 4), reinterpret_cast<const float*>(colour));
    host.gui_text_set_localised_source_00abaed0(at<void*>(marker, 4), &text, 0);
    host.widget_vcall_34_set_visible(at<void*>(marker, 4), 1);
    release_string(text, host);
}

// ---------------------------------------------------------------------------
// 005C4D40 BSP_MissionSideBlock_Destruct
// __thiscall(ECX = block), RET 0; referenced as an element destructor by
// 005C81F0, 005C89B0, 005C8AE0 and two unwind funclets (no direct calls).
// EH frame 00C72CB0 (states 13h..0 for the members). The twenty native-string
// vectors {proxy, first, last, end} at +14h + 10h*k are destroyed from k = 19
// down to 0: first non-null -> 00432050(first, last, &vector, block) and
// _free(first); then first = last = end = 0 either way. Last, the native
// string at +4h {length, data} is released.
void cc13_sweepB4_mission_side_block_destruct_005c4d40(void* block, Cc13SweepB4Host& host) {
    for (int k = 19; k >= 0; --k) {
        auto* vector = static_cast<std::uint8_t*>(block) + 0x14 + static_cast<std::size_t>(k) * 0x10;
        void* first = at<void*>(vector, 4);
        if (first != nullptr) {
            host.native_string_range_destroy_00432050(first, at<void*>(vector, 8), vector, block);
            host.crt_operator_delete_00bf65ac(at<void*>(vector, 4));
        }
        at<void*>(vector, 4) = nullptr;
        at<void*>(vector, 8) = nullptr;
        at<void*>(vector, 0xC) = nullptr;
    }
    auto& name = at<Cc13SweepBNativeString>(block, 4);
    release_string(name, host);
}

// ---------------------------------------------------------------------------
// 005EE520 BSP_HudObjectivesScreen_RefreshRows
// __thiscall(ECX = screen), RET 0. EBP frame, EH frame 00C75878 (the number
// string). Checked pointer vectors {proxy, first, last, end}: W = +40h (the
// number widgets), O = +50h (the objectives); every inlined iterator/index
// test calls 00BF6713 and carries on.
// current = O[+60h] when +60h < size(O) (unsigned), else 0; selected = -1.
// 005EDD70 erases O. manager = 008DA180(game+21A4h[game+18ECh]); two passes
// (kind 0 then 1) over its list (+4h head, nodes {next, prev, objective}):
// 008DA140(objective) and objective+18h == pass -> push_back(O) (inlined,
// 005EE3B0 on growth), selected = count when it is current, count++.
// +60h = selected != -1 ? selected : 0. 00A9BEC0(+34h). Each non-null W[k]
// gets vtable +4h(1) (deleting destructor) and the slot 0; W is cleared
// (inlined erase(begin, end)). +64h = size(O) > 0; none -> +65h = 1, done.
// x = (float)(00AA6750(+2Ch)[0] + (float)(+2Ch.+114h / 960.0)), y = (float)
// (00AA6750(+2Ch)[1] + 0.01 (double 00D7A358)); number = 1. For each
// objective i: the first one whose +18h == 1 (index checked) moves x to
// (float)(00AA6750(+30h)[0] + (float)(+30h.+114h / 960.0)) and resets number
// to 1 (once). a = 00AAB4C0(+38h, 0), 00AA8240(a, {x, y, -990.0f}),
// 00ABAED0(a, 004260B0(number), 0), push_back(W, a) (005DABA0 on growth);
// b = 00AAB4C0(+38h, 0), 00AA8240(b, {x, y, -990.0f}), 00ABBE50(b, "", 0),
// 00A9D750(+34h, b, 0, 0); x = (float)(00AA6740(b)[0] * 0.666 (double
// 00CF2D58) + x); number++. Then 00A9C7C0(+34h, +60h), +34h->+80h(1), +65h = 1.
void cc13_sweepB4_hud_objectives_refresh_rows_005ee520(void* screen, Cc13SweepB4Host& host) {
    auto check = [&](bool ok) {
        if (!ok) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    auto* const objectives = static_cast<std::uint8_t*>(screen) + 0x50;
    auto* const widgets = static_cast<std::uint8_t*>(screen) + 0x40;
    auto first = [](void* v) -> void**& { return at<void**>(v, 4); };
    auto last = [](void* v) -> void**& { return at<void**>(v, 8); };
    auto end = [](void* v) -> void**& { return at<void**>(v, 0xC); };
    auto size_of = [&](void* v) -> std::uint32_t {
        return first(v) == nullptr ? 0u : static_cast<std::uint32_t>(last(v) - first(v));
    };
    auto checked_at = [&](void* v, std::uint32_t index) -> void* {
        check(first(v) != nullptr && index < size_of(v));
        return first(v)[index];
    };

    void* current = nullptr;
    const std::uint32_t current_index = at<std::uint32_t>(screen, 0x60);
    if (size_of(objectives) > current_index) {
        current = checked_at(objectives, current_index);
    }
    std::int32_t selected = -1;
    {
        void** const stop = last(objectives);
        check(first(objectives) <= stop);
        void** const start = first(objectives);
        check(start <= last(objectives));
        void* out[2];
        host.objective_vector_erase_range_005edd70(objectives, out, objectives, start, objectives, stop);
    }

    void* game = host.global_00e188a8();
    void* owner = at<void*>(game, 0x21A4 + static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)) * 4);
    void* manager = host.objective_manager_008da180(owner);
    std::int32_t count = 0;
    for (std::int32_t pass = 0; pass <= 1; ++pass) {
        for (void* node = at<void*>(at<void*>(manager, 4), 0); node != at<void*>(manager, 4);) {
            void* objective = at<void*>(node, 8);
            if (host.objective_is_shown_in_hud_008da140(objective) && at<std::int32_t>(objective, 0x18) == pass) {
                void** const o_first = first(objectives);
                if (o_first != nullptr && size_of(objectives) < static_cast<std::uint32_t>(end(objectives) - o_first)) {
                    *last(objectives) = objective;
                    last(objectives) += 1;
                } else {
                    void** const where = last(objectives);
                    check(o_first <= where);
                    void* out[2];
                    host.objective_vector_insert_one_005ee3b0(objectives, out, objectives, where, &objective);
                }
                if (objective == current) {
                    selected = count;
                }
                ++count;
            }
            check(node != at<void*>(manager, 4));
            node = at<void*>(node, 0);
        }
    }
    at<std::int32_t>(screen, 0x60) = selected != -1 ? selected : 0;
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x34));

    for (std::uint32_t k = 0; first(widgets) != nullptr && k < size_of(widgets); ++k) {
        check(first(widgets) != nullptr && k < size_of(widgets));
        void** slot = first(widgets) + k;
        if (*slot != nullptr) {
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
    }
    {
        void** const end_it = last(widgets);
        check(first(widgets) <= end_it);
        void** const begin_it = first(widgets);
        check(begin_it <= last(widgets));
        if (begin_it != end_it) {
            const std::int32_t tail = static_cast<std::int32_t>(last(widgets) - end_it);
            void** const new_last = begin_it + tail;
            if (tail > 0) {
                host.crt_memmove_s_00bf67a7(begin_it, static_cast<std::uint32_t>(tail) * 4, end_it,
                                            static_cast<std::uint32_t>(tail) * 4);
            }
            last(widgets) = new_last;
        }
    }

    const std::int32_t rows = static_cast<std::int32_t>(size_of(objectives));
    at<std::uint8_t>(screen, 0x64) = rows > 0 ? 1 : 0;
    if (rows > 0) {
        float resolved[3];
        const float scale = static_cast<float>(at<float>(at<void*>(screen, 0x2C), 0x114) / 960.0);
        float x = static_cast<float>(
            static_cast<double>(host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x2C), resolved)[0]) +
            scale);
        const float y = static_cast<float>(
            static_cast<double>(host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x2C), resolved)[1]) +
            f64_bits(0x3F847AE140000000ull));
        bool looking = true;
        std::int32_t number = 1;
        for (std::int32_t i = 0; i < rows; ++i) {
            if (looking && at<std::int32_t>(checked_at(objectives, static_cast<std::uint32_t>(i)), 0x18) == 1) {
                const float scale2 = static_cast<float>(at<float>(at<void*>(screen, 0x30), 0x114) / 960.0);
                float resolved2[3];
                x = static_cast<float>(
                    static_cast<double>(
                        host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x30), resolved2)[0]) +
                    scale2);
                number = 1;
                looking = false;
            }
            void* row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x38), nullptr);
            const float place[3] = {x, y, -990.0f};
            host.widget_set_resolved_position_00aa8240(row, place);
            Cc13SweepBNativeString text;
            host.native_string_from_int_004260b0(&text, number);
            host.gui_text_set_localised_source_00abaed0(row, &text, 0);
            release_string(text, host);
            void** const w_first = first(widgets);
            if (w_first != nullptr && size_of(widgets) < static_cast<std::uint32_t>(end(widgets) - w_first)) {
                *last(widgets) = row;
                last(widgets) += 1;
            } else {
                void** const where = last(widgets);
                check(w_first <= where);
                void* out[2];
                host.widget_vector_insert_one_005daba0(widgets, out, widgets, where, &row);
            }
            void* spacer = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x38), nullptr);
            const float place2[3] = {x, y, -990.0f};
            host.widget_set_resolved_position_00aa8240(spacer, place2);
            host.gui_text_set_localised_cstring_00abbe50(spacer, static_cast<const char*>(host.image_global(0x00CE3A0C)),
                                                         0);
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x34), spacer, nullptr, 0);
            x = static_cast<float>(static_cast<double>(host.gui_widget_get_size_00aa6740(spacer)[0]) *
                                       f64_bits(0x3FE54FDF40000000ull) +
                                   x);
            ++number;
        }
        host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x34), at<std::int32_t>(screen, 0x60));
        host.widget_vcall_80(at<void*>(screen, 0x34), 1);
    }
    at<std::uint8_t>(screen, 0x65) = 1;
}

// ---------------------------------------------------------------------------
// 00536310 BSP_HudFormationScreen_BuildQuadModel
// __thiscall(ECX = screen; float size), RET 4. EH frame 00C6C2C2.
// Same shape as cc13_sweepB3's 005A2340: model = 00B75030(00B74EB0(),
// 0043C130("MeshQuad", "FormationGuikvad")) when the pool block is non-null
// (else 0) -> screen+174h; the two names released (the prefixed one first,
// each only on the path that built it). geometry = 00B73D70(00B73B60()) or
// 0. resource = renderer vtable +38h("pf43.mvfm"); stream = +5Ch(4, 1,
// resource); resource dropped WITHOUT a null check; 00B73BB0(geometry, 0,
// stream); stream dropped; stream vtable +10h(0, 0, 0) gets the quad {n,0,p,
// p,0,p, n,0,n, p,0,n} with n = (float)(-size * 0.5), p = (float)(size *
// 0.5). section = 00533FA0(); material = 00535320("formation_gui.mshd");
// 00B18B20(material, "cCircleRadiusSquared", screen+184h); 00B18B00(material,
// "cCirclePosition", screen+17Ch); section +8h/+10h/+18h = 5/4/2; stream
// +14h(); 00B179F0(material, 0) = {1,1,1,1}; 00B864C0(section, material);
// material dropped; 00B865A0(section, geometry); 00B73C60(geometry,
// section); section dropped; 00B75170(+174h, 0, geometry, -1.0f, -1.0f);
// geometry dropped; (+174h)->+30h({0, 1400.0f, 0}); 00B6D890(+174h,
// screen+0Ch). The names of the inline strings are released after each use.
void cc13_sweepB4_hud_formation_build_quad_model_00536310(void* screen, float size, Cc13SweepB4Host& host) {
    auto drop = [&](void* object) {  // the image's unchecked reference release
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    auto literal = [&](Cc13SweepBNativeString& text, std::uint32_t length, std::uint32_t address) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, host.image_global(address), text.length + 1);
        }
    };

    void* model = nullptr;
    void* model_block = host.generated_model_pool_alloc_00b74eb0();
    if (model_block != nullptr) {
        Cc13SweepBNativeString suffix;
        host.native_string_assign_0041e870(&suffix, static_cast<const char*>(host.image_global(0x00CED5B8)));
        Cc13SweepBNativeString name;
        host.native_string_prefixed_0043c130(&name, static_cast<const char*>(host.image_global(0x00CED5AC)), &suffix);
        model = host.generated_model_construct_00b75030(model_block, &name);
        at<void*>(screen, 0x174) = model;
        release_string(name, host);
        release_string(suffix, host);
    } else {
        at<void*>(screen, 0x174) = nullptr;
    }
    void* geometry_block = host.mesh_object_pool_alloc_00b73b60();
    void* geometry = geometry_block != nullptr ? host.mesh_object_construct_00b73d70(geometry_block) : nullptr;
    Cc13SweepBNativeString resource_name;
    literal(resource_name, 9, 0x00CED5A0);  // "pf43.mvfm"
    void* resource = host.renderer_vcall_38_load(host.renderer_00f8d394(), &resource_name);
    release_string(resource_name, host);
    void* stream = host.renderer_vcall_5c_create_stream(host.renderer_00f8d394(), 4, 1, resource);
    drop(resource);
    host.geometry_set_vertex_stream_00b73bb0(geometry, 0, stream);
    drop(stream);
    float* vertices = host.stream_vcall_10_lock(stream, 0, 0, 0);
    const float n = static_cast<float>(-static_cast<double>(size) * 0.5);
    const float p = static_cast<float>(static_cast<double>(size) * 0.5);
    const float quad[12] = {n, 0.0f, p, p, 0.0f, p, n, 0.0f, n, p, 0.0f, n};
    for (std::size_t i = 0; i < 12; ++i) {
        vertices[i] = quad[i];
    }
    void* section = host.draw_section_create_00533fa0();
    Cc13SweepBNativeString material_name;
    literal(material_name, 0x12, 0x00CED58C);  // "formation_gui.mshd"
    void* material = host.material_create_for_effect_name_00535320(&material_name);
    release_string(material_name, host);
    Cc13SweepBNativeString radius_name;
    literal(radius_name, 0x14, 0x00CED574);  // "cCircleRadiusSquared"
    host.material_register_float_parameter_00b18b20(material, &radius_name, static_cast<std::uint8_t*>(screen) + 0x184);
    release_string(radius_name, host);
    Cc13SweepBNativeString position_name;
    literal(position_name, 0xF, 0x00CED564);  // "cCirclePosition"
    host.material_register_float2_parameter_00b18b00(material, &position_name, static_cast<std::uint8_t*>(screen) + 0x17C);
    release_string(position_name, host);
    at<std::uint32_t>(section, 0x8) = 5;
    at<std::uint32_t>(section, 0x10) = 4;
    at<std::uint32_t>(section, 0x18) = 2;
    host.stream_vcall_14_unlock(stream);
    float* colour = host.material_diffuse_color_00b179f0(material, 0);
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
    host.draw_section_set_material_00b864c0(section, material);
    drop(material);
    host.draw_section_rebuild_layout_00b865a0(section, geometry);
    host.geometry_append_draw_section_00b73c60(geometry, section);
    drop(section);
    host.generated_model_set_geometry_00b75170(at<void*>(screen, 0x174), 0, geometry, -1.0f, -1.0f);
    drop(geometry);
    const float lift[3] = {0.0f, 1400.0f, 0.0f};
    host.object_vcall_30_vec3(at<void*>(screen, 0x174), lift);
    host.node_propagate_root_00b6d890(at<void*>(screen, 0x174), at<void*>(screen, 0xC));
}

// ---------------------------------------------------------------------------
// 005A2730 BSP_HudMap_CreateConeModel
// __stdcall(float radius, float angle), RET 8; EAX = the model. ECX is not
// an input (it is loaded with 184h and 0BCh before the pool calls, which
// overwrite it). EH frame 00C71509. Sole caller 005A9770.
// model = 00B75030(00B74EB0(), "ACone") or 0; geometry = 00B73D70(00B73B60())
// or 0; 00B75170(model, 0, geometry, -1.0f, -1.0f) first. Vertex stream =
// renderer +5Ch(4Ah, 1, +38h("SimpleColor.mvfm")); resource dropped;
// 00B73BB0(geometry, 0, stream); stream dropped; index stream = renderer
// +60h(0D8h, 1, 65h); 00B73B70(geometry, it); dropped. Vertex lock +10h(0,0,0)
// (result unused: the writes go through the stream's own fields: base +8h,
// stride +0Ch, position offset +10h, packed colour offset +34h (>= 0) or the
// float colour offsets +38h..+44h); index lock +0Ch(0,0,0). section =
// 00533FA0(); material = 00535320("guialphawritediff.mshd"); section +8h=4,
// +0Ch=0, +14h=0, +10h=4Ah, +18h=48h. Vertex 0 = {0,0,0}; a = (float)(pi/2 -
// angle*0.5) (pi/2 the float-rounded double 00CE3830), step = (float)(angle /
// 72.0); vertices 1..73: {(float)(cos(a) * radius), (float)(radius * 0.0),
// (float)(radius * sin(a))} with sin/cos float-stored, a = (float)(step + a);
// every vertex white (0FFFFFFFFh packed or 1.0f x4). Indices: 72 fan
// triangles {0, k+1, k+2}. Unlocks (vertex +14h, index +10h); 00B864C0(section,
// material); 00B179F0(material, 0) = {1,1,1,1}, then 00B179F0(material, 0)[3]
// = 0.5f; material dropped; 00B865A0(section, geometry); 00B73C60(geometry,
// section); section and geometry dropped. Every drop is the image's
// unchecked InterlockedDecrement + vtable +0h release.
void* cc13_sweepB4_hud_map_create_cone_model_005a2730(float radius, float angle, Cc13SweepB4Host& host) {
    auto drop = [&](void* object) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    auto literal = [&](Cc13SweepBNativeString& text, std::uint32_t length, std::uint32_t address) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, host.image_global(address), text.length + 1);
        }
    };

    void* model = nullptr;
    void* model_block = host.generated_model_pool_alloc_00b74eb0();
    if (model_block != nullptr) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(0x00CF008C)));
        model = host.generated_model_construct_00b75030(model_block, &name);
        release_string(name, host);
    }
    void* geometry_block = host.mesh_object_pool_alloc_00b73b60();
    void* geometry = geometry_block != nullptr ? host.mesh_object_construct_00b73d70(geometry_block) : nullptr;
    host.generated_model_set_geometry_00b75170(model, 0, geometry, -1.0f, -1.0f);

    Cc13SweepBNativeString resource_name;
    literal(resource_name, 0x10, 0x00CF0078);  // "SimpleColor.mvfm"
    void* resource = host.renderer_vcall_38_load(host.renderer_00f8d394(), &resource_name);
    release_string(resource_name, host);
    void* vertices = host.renderer_vcall_5c_create_stream(host.renderer_00f8d394(), 0x4A, 1, resource);
    drop(resource);
    host.geometry_set_vertex_stream_00b73bb0(geometry, 0, vertices);
    drop(vertices);
    void* indices = host.renderer_vcall_60_create_index_stream(host.renderer_00f8d394(), 0xD8, 1, 0x65);
    host.geometry_set_index_stream_00b73b70(geometry, indices);
    drop(indices);
    host.stream_vcall_10_lock(vertices, 0, 0, 0);
    std::uint16_t* index_data = host.index_stream_vcall_0c_lock(indices, 0, 0, 0);
    void* section = host.draw_section_create_00533fa0();
    Cc13SweepBNativeString material_name;
    literal(material_name, 0x16, 0x00CF0060);  // "guialphawritediff.mshd"
    void* material = host.material_create_for_effect_name_00535320(&material_name);
    release_string(material_name, host);
    at<std::uint32_t>(section, 0x8) = 4;
    at<std::uint32_t>(section, 0xC) = 0;
    at<std::uint32_t>(section, 0x14) = 0;
    at<std::uint32_t>(section, 0x10) = 0x4A;
    at<std::uint32_t>(section, 0x18) = 0x48;

    auto* const base = at<std::uint8_t*>(vertices, 8);
    auto vertex = [&](std::int32_t i) {
        return base + static_cast<std::size_t>(at<std::int32_t>(vertices, 0xC) * i);
    };
    auto paint = [&](std::int32_t i) {
        const std::int32_t packed = at<std::int32_t>(vertices, 0x34);
        if (packed >= 0) {
            at<std::uint32_t>(vertex(i), static_cast<std::size_t>(packed)) = 0xFFFFFFFFu;
        } else {
            for (std::size_t k = 0; k < 4; ++k) {
                at<float>(vertex(i), static_cast<std::size_t>(at<std::int32_t>(vertices, 0x38 + k * 4))) = 1.0f;
            }
        }
    };
    {
        float* p = reinterpret_cast<float*>(vertex(0) + at<std::int32_t>(vertices, 0x10));
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
        paint(0);
    }
    float a = static_cast<float>(f64_bits(0x3FF921FB60000000ull) - static_cast<double>(angle) * 0.5);
    const float step = static_cast<float>(static_cast<double>(angle) / 72.0);
    const float y = static_cast<float>(static_cast<double>(radius) * 0.0);
    for (std::int32_t i = 1; i <= 0x49; ++i) {
        const float s = static_cast<float>(std::sin(static_cast<double>(a)));
        const float c = static_cast<float>(std::cos(static_cast<double>(a)));
        float* p = reinterpret_cast<float*>(vertex(i) + at<std::int32_t>(vertices, 0x10));
        p[0] = static_cast<float>(static_cast<double>(c) * radius);
        p[1] = y;
        p[2] = static_cast<float>(static_cast<double>(radius) * s);
        paint(i);
        a = static_cast<float>(static_cast<double>(step) + a);
    }
    for (std::uint32_t k = 0; k < 0x48; ++k) {
        index_data[k * 3] = 0;
        index_data[k * 3 + 1] = static_cast<std::uint16_t>(k + 1);
        index_data[k * 3 + 2] = static_cast<std::uint16_t>(k + 2);
    }
    host.stream_vcall_14_unlock(vertices);
    host.index_stream_vcall_10_unlock(indices);
    host.draw_section_set_material_00b864c0(section, material);
    float* colour = host.material_diffuse_color_00b179f0(material, 0);
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
    host.material_diffuse_color_00b179f0(material, 0)[3] = 0.5f;
    drop(material);
    host.draw_section_rebuild_layout_00b865a0(section, geometry);
    host.geometry_append_draw_section_00b73c60(geometry, section);
    drop(section);
    drop(geometry);
    return model;
}

// ---------------------------------------------------------------------------
// 0063C110 BSP_HudUnitMarkerWidgets_Construct
// __thiscall(ECX = set; root), RET 4; EAX = set. EBP frame, EH frame
// 00C79F98 (the name temporaries, each built inline: {0,0}, 0041DD40(length,
// 1), memcpy of length + 1 when data is non-null).
// +0h = vtable 00CF5758 (stored as the image address value). close =
// 00AA7E00(root, "CLOSE_Group", 1). For each of the six colour names
// [00E08C90..00E08CA4] (Red_Icon first), four times with the SAME name:
// slot = 00AAB4C0(00AA7E00(close, name, 1), 0) into +8h.. (24 slots, +8h..
// +64h), the name released, then slot->+34h(0). Then, under root, each found
// widget is cloned AFTER its name is released (00AAB4C0(found, 0)):
// HPCsik_Icon -> +68h, HPCsikBack_Icon -> +6Ch, DeviceMarker_Text -> +78h,
// DistanceMarker_Text -> +7Ch, type_Icon -> +70h, sidemarker_Group -> +80h,
// commandbuilding_Group -> +74h (not hidden).
void* cc13_sweepB4_hud_unit_marker_widgets_construct_0063c110(void* set, void* root, Cc13SweepB4Host& host) {
    auto name_of = [&](Cc13SweepBNativeString& text, const char* source) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(source)), 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
        }
    };
    auto image_text = [&](std::uint32_t address) { return static_cast<const char*>(host.image_global(address)); };

    at<std::uint32_t>(set, 0) = 0x00CF5758u;
    void* close;
    {
        Cc13SweepBNativeString name;
        name_of(name, image_text(0x00CF5804));  // "CLOSE_Group"
        close = host.widget_find_child_00aa7e00(root, &name, 1);
        release_string(name, host);
    }
    std::size_t slot_offset = 8;
    for (std::uint32_t entry = 0x00E08C90; entry < 0x00E08CA8; entry += 4) {
        for (int copy = 0; copy < 4; ++copy) {
            const char* colour = *static_cast<const char**>(host.image_global(entry));
            Cc13SweepBNativeString name;
            name_of(name, colour);
            void* found = host.widget_find_child_00aa7e00(close, &name, 1);
            at<void*>(set, slot_offset) = host.widget_clone_subtree_00aab4c0(found, nullptr);
            release_string(name, host);
            host.widget_vcall_34_set_visible(at<void*>(set, slot_offset), 0);
            slot_offset += 4;
        }
    }
    struct Child {
        std::uint32_t name;
        std::size_t field;
    };
    static const Child kChildren[] = {
        {0x00CF57F8, 0x68}, {0x00CF57E8, 0x6C}, {0x00CF57D4, 0x78}, {0x00CF57C0, 0x7C},
        {0x00CF57B4, 0x70}, {0x00CF57A0, 0x80}, {0x00CF5788, 0x74},
    };
    for (const Child& child : kChildren) {
        Cc13SweepBNativeString name;
        name_of(name, image_text(child.name));
        void* found = host.widget_find_child_00aa7e00(root, &name, 1);
        release_string(name, host);
        at<void*>(set, child.field) = host.widget_clone_subtree_00aab4c0(found, nullptr);
    }
    return set;
}

// ---------------------------------------------------------------------------
// 005D43C0 BSP_HudPauseMultiScreen_OnConfirm
// __thiscall(ECX = screen; widget, arg2), RET 8; listener slot (00CF1D60), no
// direct callers. EBP frame, EH frame 00C73FE8 (the string temporaries).
// key = widget+0F4h (a native string). The first row key [00E0889C]
// ("^FE.multi_ingame_resume") is compared INLINE: key data null -> equal when
// the C string is null or empty; key data set -> equal when the C string is
// null and the key length is 0, or _stricmp == 0. Equal -> 004DB030(game),
// [00E19550] = 0. The others go through 00425850 in this order:
// [00E088A8] "^FE.multi_ingame_points" -> 00688AD0([00E198B4], 1Fh);
// [00E088AC] "ingame.pause_options" -> [00E19650] = 0, [00E198B8]->+8h();
// [00E088A4] "^ingame.pause_tips" -> pause = [00E198C4]+0D8h: 00601200(pause),
// byte pause+10h = 1, 005FD890(pause, 1, 1), +0Ch/+10h hidden;
// [00E088A0] "^FE.multi_ingame_players" -> 00688AD0([00E198B4], 16h);
// [00E088B0] "^ingame.pause_surrender" -> prompt(4, "ingame.pause_surrender_yesno",
//   callback 005D36B0);
// [00E088B4] "^ingame.pause_notsurrender" -> 00771F20(game+1EF0h, game+18C8h,
//   0), byte [00E1954A] = 0, byte [00E19549] = 1, 005D39A0(ECX = arg2, EDX = 5);
// [00E088B8] "^FE.multi_ingame_quit" -> T = "globals.quit|.\n|" (+
//   "globals.willendsession|.\n|" when 004BB8A0(game) and byte game+218Ch is
//   clear) + "globals.areyousurequit", prompt(4, T, callback 005D3410);
// [00E088BC] "^FE.multi_ingame_exitprogram" -> T = "globals.exitprogram|.\n|"
//   (+ "globals.willendsession" under the same test) +
//   "|.\n|globals.areyousureexit", prompt(6, T, callback 005D3430).
// prompt(type, message, callback): title = "" (0041E870), a by-value ""
// built in the argument slot, 00531B00(00425D10(), type, &message, 1,
// callback, 0, &title, 0.0f, 0, by-value "", 1); then the temporaries are
// destroyed through 0041DD20 and 00530C20(00425D10()) runs (the surrender
// branch destroys message then title before 00530C20; quit/exit destroy the
// title, call 00530C20, then destroy T).
void cc13_sweepB4_hud_pause_multi_on_confirm_005d43c0(void* screen, void* widget, void* arg2,
                                                      Cc13SweepB4Host& host) {
    auto* key = static_cast<std::uint8_t*>(widget) + 0xF4;
    auto image_text = [&](std::uint32_t address) { return static_cast<const char*>(host.image_global(address)); };
    auto row_key = [&](std::uint32_t slot) { return *static_cast<const char**>(host.image_global(slot)); };
    auto matches = [&](std::uint32_t slot) { return host.native_string_equals_cstring_ci_00425850(key, row_key(slot)); };

    bool resume;
    {
        const char* resume_key = row_key(0x00E0889C);
        if (at<void*>(key, 4) == nullptr) {
            resume = resume_key == nullptr || std::strlen(resume_key) == 0;
        } else if (resume_key == nullptr) {
            resume = at<std::uint32_t>(key, 0) == 0;
        } else {
            resume = host.crt_stricmp_00bf7fbf(static_cast<const char*>(at<void*>(key, 4)), resume_key) == 0;
        }
    }
    if (resume) {
        host.game_toggle_pause_menu_004db030(host.global_00e188a8());
        *static_cast<std::uint32_t*>(host.image_global(0x00E19550)) = 0;
        return;
    }
    if (matches(0x00E088A8)) {
        host.multi_menu_push_request_interface_00688ad0(*static_cast<void**>(host.image_global(0x00E198B4)), 0x1F);
        return;
    }
    if (matches(0x00E088AC)) {
        *static_cast<std::uint32_t*>(host.image_global(0x00E19650)) = 0;
        host.screen_vcall_08(*static_cast<void**>(host.image_global(0x00E198B8)));
        return;
    }
    if (matches(0x00E088A4)) {
        auto pause = [&]() { return at<void*>(*static_cast<void**>(host.image_global(0x00E198C4)), 0xD8); };
        host.hud_pause_build_hint_list_00601200(pause());
        at<std::uint8_t>(pause(), 0x10) = 1;
        host.hud_pause_show_page_005fd890(pause(), 1, 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xC), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x10), 0);
        return;
    }
    if (matches(0x00E088A0)) {
        host.multi_menu_push_request_interface_00688ad0(*static_cast<void**>(host.image_global(0x00E198B4)), 0x16);
        return;
    }
    // The by-value "" argument and the 00531B00 call shared by the prompts.
    auto raise = [&](std::int32_t type, const void* message, std::uint32_t callback, const void* title) {
        Cc13SweepBNativeString by_value;
        host.native_string_assign_0041e870(&by_value, image_text(0x00CE3A0C));
        void* menu = host.menu_command_screen_00425d10();
        host.menu_prompt_raise_00531b00(menu, type, message, 1, callback, 0, title, 0.0f, 0, by_value, 1);
    };
    auto build_question = [&](Cc13SweepBNativeString& text, std::uint32_t head, std::uint32_t warning,
                              std::uint32_t tail) {
        host.native_string_assign_0041e870(&text, image_text(head));
        void* game_now = host.global_00e188a8();
        if (host.game_session_local_side_check_004bb8a0(game_now) &&
            at<std::uint8_t>(host.global_00e188a8(), 0x218C) == 0) {
            Cc13SweepBNativeString extra;
            host.native_string_assign_0041e870(&extra, image_text(warning));
            host.native_string_append_00425e10(&text, &extra);
            host.native_string_destroy_storage_0041dd20(&extra);
        }
        Cc13SweepBNativeString extra;
        host.native_string_assign_0041e870(&extra, image_text(tail));
        host.native_string_append_00425e10(&text, &extra);
        host.native_string_destroy_storage_0041dd20(&extra);
    };
    if (matches(0x00E088B0)) {
        Cc13SweepBNativeString title;
        host.native_string_assign_0041e870(&title, image_text(0x00CE3A0C));
        Cc13SweepBNativeString message;
        host.native_string_assign_0041e870(&message, image_text(0x00CF1E6C));  // "ingame.pause_surrender_yesno"
        raise(4, &message, 0x005D36B0u, &title);
        host.native_string_destroy_storage_0041dd20(&message);
        host.native_string_destroy_storage_0041dd20(&title);
        host.frontend_prompt_focus_last_navigation_00530c20(host.menu_command_screen_00425d10());
        return;
    }
    if (matches(0x00E088B4)) {
        void* game_now = host.global_00e188a8();
        host.session_request_player_surrender_00771f20(static_cast<std::uint8_t*>(game_now) + 0x1EF0,
                                                       at<std::int32_t>(game_now, 0x18C8), 0);
        *static_cast<std::uint8_t*>(host.image_global(0x00E1954A)) = 0;
        *static_cast<std::uint8_t*>(host.image_global(0x00E19549)) = 1;
        host.hud_pause_multi_build_menu_rows_005d39a0(arg2, 5);
        return;
    }
    if (matches(0x00E088B8)) {
        Cc13SweepBNativeString question;
        build_question(question, 0x00CF1E58, 0x00CE7B80, 0x00CF1E40);
        Cc13SweepBNativeString title;
        host.native_string_assign_0041e870(&title, image_text(0x00CE3A0C));
        raise(4, &question, 0x005D3410u, &title);
        host.native_string_destroy_storage_0041dd20(&title);
        host.frontend_prompt_focus_last_navigation_00530c20(host.menu_command_screen_00425d10());
        host.native_string_destroy_storage_0041dd20(&question);
        return;
    }
    if (matches(0x00E088BC)) {
        Cc13SweepBNativeString question;
        build_question(question, 0x00CF1E28, 0x00CF1E10, 0x00CF1DF4);
        Cc13SweepBNativeString title;
        host.native_string_assign_0041e870(&title, image_text(0x00CE3A0C));
        raise(6, &question, 0x005D3430u, &title);
        host.native_string_destroy_storage_0041dd20(&title);
        host.frontend_prompt_focus_last_navigation_00530c20(host.menu_command_screen_00425d10());
        host.native_string_destroy_storage_0041dd20(&question);
    }
}

// ---------------------------------------------------------------------------
// 005A2C30 BSP_HudMap_CreateSimpleColorModel (a filled circle)
// __stdcall(float radius), RET 4; EAX = the model. ECX is not an input. EH
// frame as 005A2730, whose shape this repeats with: model name "ACircle",
// 49h vertices, section +10h = 49h. Vertex 0 = centre (colour written before
// the position here), vertex 1 = {(float)((float)sin(0) * radius), 0,
// (float)((float)cos(0) * radius)}; vertices i = 2..72 at t = (float)((i - 1)
// * 0.0872664675 (5 degrees, double 00CF0098)) with x = (float)((float)sin(t)
// * radius), z = (float)((float)cos(t) * radius); all white. Indices: {0,
// i-1, i} for i = 2..72, then the closing {0, 72, 1} at index 213. Unlock
// order: index stream +10h, then vertex stream +14h. Tail as 005A2730
// (diffuse {1,1,1,1}, alpha 0.5, unchecked drops).
void* cc13_sweepB4_hud_map_create_circle_model_005a2c30(float radius, Cc13SweepB4Host& host) {
    auto drop = [&](void* object) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    auto literal = [&](Cc13SweepBNativeString& text, std::uint32_t length, std::uint32_t address) {
        text = {0, nullptr};
        host.native_string_resize_0041dd40(&text, length, 1);
        if (text.data != nullptr) {
            host.crt_memcpy_00bf7680(text.data, host.image_global(address), text.length + 1);
        }
    };

    void* model = nullptr;
    void* model_block = host.generated_model_pool_alloc_00b74eb0();
    if (model_block != nullptr) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(0x00CF00A0)));
        model = host.generated_model_construct_00b75030(model_block, &name);
        release_string(name, host);
    }
    void* geometry_block = host.mesh_object_pool_alloc_00b73b60();
    void* geometry = geometry_block != nullptr ? host.mesh_object_construct_00b73d70(geometry_block) : nullptr;
    host.generated_model_set_geometry_00b75170(model, 0, geometry, -1.0f, -1.0f);

    Cc13SweepBNativeString resource_name;
    literal(resource_name, 0x10, 0x00CF0078);  // "SimpleColor.mvfm"
    void* resource = host.renderer_vcall_38_load(host.renderer_00f8d394(), &resource_name);
    release_string(resource_name, host);
    void* vertices = host.renderer_vcall_5c_create_stream(host.renderer_00f8d394(), 0x49, 1, resource);
    drop(resource);
    host.geometry_set_vertex_stream_00b73bb0(geometry, 0, vertices);
    drop(vertices);
    void* indices = host.renderer_vcall_60_create_index_stream(host.renderer_00f8d394(), 0xD8, 1, 0x65);
    host.geometry_set_index_stream_00b73b70(geometry, indices);
    drop(indices);
    host.stream_vcall_10_lock(vertices, 0, 0, 0);
    std::uint16_t* index_data = host.index_stream_vcall_0c_lock(indices, 0, 0, 0);
    void* section = host.draw_section_create_00533fa0();
    Cc13SweepBNativeString material_name;
    literal(material_name, 0x16, 0x00CF0060);  // "guialphawritediff.mshd"
    void* material = host.material_create_for_effect_name_00535320(&material_name);
    release_string(material_name, host);
    at<std::uint32_t>(section, 0xC) = 0;
    at<std::uint32_t>(section, 0x14) = 0;
    at<std::uint32_t>(section, 0x8) = 4;
    at<std::uint32_t>(section, 0x10) = 0x49;
    at<std::uint32_t>(section, 0x18) = 0x48;

    auto* const base = at<std::uint8_t*>(vertices, 8);
    auto vertex = [&](std::int32_t i) {
        return base + static_cast<std::size_t>(at<std::int32_t>(vertices, 0xC) * i);
    };
    auto paint = [&](std::int32_t i) {
        const std::int32_t packed = at<std::int32_t>(vertices, 0x34);
        if (packed >= 0) {
            at<std::uint32_t>(vertex(i), static_cast<std::size_t>(packed)) = 0xFFFFFFFFu;
        } else {
            for (std::size_t k = 0; k < 4; ++k) {
                at<float>(vertex(i), static_cast<std::size_t>(at<std::int32_t>(vertices, 0x38 + k * 4))) = 1.0f;
            }
        }
    };
    auto place = [&](std::int32_t i, float x, float z) {
        float* p = reinterpret_cast<float*>(vertex(i) + at<std::int32_t>(vertices, 0x10));
        p[0] = x;
        p[1] = 0.0f;
        p[2] = z;
    };
    paint(0);
    place(0, 0.0f, 0.0f);
    {
        const float s = static_cast<float>(std::sin(0.0));
        const float x = static_cast<float>(static_cast<double>(s) * radius);
        const float c = static_cast<float>(std::cos(0.0));
        const float z = static_cast<float>(static_cast<double>(c) * radius);
        paint(1);
        place(1, x, z);
    }
    const double step = f64_bits(0x3FB6571860000000ull);  // 00CF0098
    for (std::int32_t i = 2; i <= 0x48; ++i) {
        const float t = static_cast<float>(static_cast<double>(i - 1) * step);
        const float s = static_cast<float>(std::sin(static_cast<double>(t)));
        const float x = static_cast<float>(static_cast<double>(s) * radius);
        const float c = static_cast<float>(std::cos(static_cast<double>(t)));
        const float z = static_cast<float>(static_cast<double>(c) * radius);
        place(i, x, z);
        paint(i);
        const std::size_t k = static_cast<std::size_t>(i - 2) * 3;
        index_data[k] = 0;
        index_data[k + 1] = static_cast<std::uint16_t>(i - 1);
        index_data[k + 2] = static_cast<std::uint16_t>(i);
    }
    index_data[0x1AC / 2] = 0x48;
    index_data[0x1AA / 2] = 0;
    index_data[0x1AE / 2] = 1;
    host.index_stream_vcall_10_unlock(indices);
    host.stream_vcall_14_unlock(vertices);
    host.draw_section_set_material_00b864c0(section, material);
    float* colour = host.material_diffuse_color_00b179f0(material, 0);
    colour[0] = 1.0f;
    colour[1] = 1.0f;
    colour[2] = 1.0f;
    colour[3] = 1.0f;
    host.material_diffuse_color_00b179f0(material, 0)[3] = 0.5f;
    drop(material);
    host.draw_section_rebuild_layout_00b865a0(section, geometry);
    host.geometry_append_draw_section_00b73c60(geometry, section);
    drop(section);
    drop(geometry);
    return model;
}

// ---------------------------------------------------------------------------
// 005A1C60 BSP_HudMapIcon_ReleaseWidgets
// __thiscall(ECX = icon; int keep_first (low byte tested)), RET 4. No EH frame.
// Checked pointer vectors {proxy, first, last, end}. For +0C8h and +0D8h:
// every non-null slot gets vtable +4h(1) (deleting destructor) and is
// zeroed, then the vector is cleared (inlined erase(begin, end) with
// memmove_s of the zero-length tail). +0E8h: the same deletes, except that
// slot 0 is skipped when keep_first is set; after each delete the slot is
// re-checked and zeroed again; the vector is NOT cleared. keep_first clear
// -> +118h = 0. Then +88h, +78h, +0A8h, +98h, +0B8h as +0C8h. Every index
// and iterator test calls 00BF6713 and carries on.
void cc13_sweepB4_hud_map_icon_release_widgets_005a1c60(void* icon, std::int32_t keep_first, Cc13SweepB4Host& host) {
    auto check = [&](bool ok) {
        if (!ok) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    auto* const base = static_cast<std::uint8_t*>(icon);
    auto first = [&](std::size_t v) -> void**& { return at<void**>(base + v, 4); };
    auto last = [&](std::size_t v) -> void**& { return at<void**>(base + v, 8); };
    auto size_of = [&](std::size_t v) -> std::uint32_t {
        return static_cast<std::uint32_t>(last(v) - first(v));
    };
    auto delete_all = [&](std::size_t v) {
        for (std::uint32_t k = 0; first(v) != nullptr && k < size_of(v); ++k) {
            check(first(v) != nullptr && k < size_of(v));
            void** slot = first(v) + k;
            if (*slot != nullptr) {
                host.object_vcall_04_delete(*slot, 1);
                *slot = nullptr;
            }
        }
    };
    auto clear = [&](std::size_t v) {
        void** const end_it = last(v);
        check(first(v) <= end_it);
        void** const begin_it = first(v);
        check(begin_it <= last(v));
        if (begin_it == end_it) {
            return;
        }
        const std::int32_t tail = static_cast<std::int32_t>(last(v) - end_it);
        void** const new_last = begin_it + tail;
        if (tail > 0) {
            host.crt_memmove_s_00bf67a7(begin_it, static_cast<std::uint32_t>(tail) * 4, end_it,
                                        static_cast<std::uint32_t>(tail) * 4);
        }
        last(v) = new_last;
    };

    delete_all(0xC8);
    clear(0xC8);
    delete_all(0xD8);
    clear(0xD8);
    const bool keep = (keep_first & 0xFF) != 0;
    for (std::int32_t k = 0; first(0xE8) != nullptr && static_cast<std::uint32_t>(k) < size_of(0xE8); ++k) {
        if (keep && k < 1) {
            continue;
        }
        check(first(0xE8) != nullptr && static_cast<std::uint32_t>(k) < size_of(0xE8));
        void** slot = first(0xE8) + k;
        if (*slot != nullptr) {
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
        check(first(0xE8) != nullptr && static_cast<std::uint32_t>(k) < size_of(0xE8));
        first(0xE8)[k] = nullptr;
    }
    if (!keep) {
        at<std::int32_t>(icon, 0x118) = 0;
    }
    for (std::size_t v : {std::size_t{0x88}, std::size_t{0x78}, std::size_t{0xA8}, std::size_t{0x98}, std::size_t{0xB8}}) {
        delete_all(v);
        clear(v);
    }
}

// ---------------------------------------------------------------------------
// 00652C80 BSP_Hud_UnitIconFrame_00652C80
// __thiscall(ECX = screen; class, a2, a3, char use_unit, unit), RET 14h; EAX.
// a2/a3 are not read. No EH frame. Sibling of 00653000 (which takes only the
// unit and has no class path).
// use_unit set: unit->+5Ch(18h) -> when unit->+174h() is the class singleton
// 00E08FA0: game mode (004BCA50) 4..7 -> byte screen+6Ch = 1, else byte
// screen+6Ch+[screen+68h] = 1; byte screen+1DCh clear -> 21h. Then the
// classifier c = unit+35Ch through c->+18h(kind): 12h -> 0Eh, 13h -> 0Fh,
// 10h -> 11h, 11h -> 13h, 16h/15h -> 10h, 17h -> 0Fh. Otherwise unit->+5Ch:
// 0Eh -> 0Dh, 7 -> 4, 0Dh -> byte (unit+538h)+808h ? 15h : 0Ch, 0Ah -> 7 +
// (byte +808h != 0), 8 -> 5, 0Bh -> 9, 0Ch -> byte +81Ah ? 0Ah : 0Bh, 9 ->
// byte +82Ch ? 16h : 6, else 3.
// use_unit clear: class->+18h: 12h -> 0Eh, 13h -> 0Fh, 10h -> 11h, 11h -> 13h,
// 16h/15h -> 10h, 0Eh -> 0Dh, 7 -> 4, 0Dh/0Ah/8/0Bh/0Ch/9 as above but with
// the bytes read at class+808h/+81Ah/+82Ch directly (no 17h test), else 3.
std::int32_t cc13_sweepB4_hud_unit_icon_frame_00652c80(void* screen, void* class_object, std::int32_t unused2,
                                                       std::int32_t unused3, std::int32_t use_unit, void* unit,
                                                       Cc13SweepB4Host& host) {
    (void)unused2;
    (void)unused3;
    if ((use_unit & 0xFF) != 0) {
        if (host.object_vcall_5c_is_kind(unit, 0x18)) {
            if (host.unit_vcall_174_command_class(unit) == host.image_global(0x00E08FA0)) {
                const std::int32_t mode = host.game_effective_mode_004bca50(host.global_00e188a8());
                if (mode == 4 || mode == 5 || mode == 6 || mode == 7) {
                    at<std::uint8_t>(screen, 0x6C) = 1;
                } else {
                    at<std::uint8_t>(screen, 0x6C + static_cast<std::size_t>(at<std::int32_t>(screen, 0x68))) = 1;
                }
                if (at<std::uint8_t>(screen, 0x1DC) == 0) {
                    return 0x21;
                }
            }
            void* kind = at<void*>(unit, 0x35C);
            if (host.class_vcall_18_is_kind(kind, 0x12)) return 0xE;
            if (host.class_vcall_18_is_kind(kind, 0x13)) return 0xF;
            if (host.class_vcall_18_is_kind(kind, 0x10)) return 0x11;
            if (host.class_vcall_18_is_kind(kind, 0x11)) return 0x13;
            if (host.class_vcall_18_is_kind(kind, 0x16)) return 0x10;
            if (host.class_vcall_18_is_kind(kind, 0x15)) return 0x10;
            if (host.class_vcall_18_is_kind(kind, 0x17)) return 0xF;
        }
        if (host.object_vcall_5c_is_kind(unit, 0xE)) return 0xD;
        if (host.object_vcall_5c_is_kind(unit, 7)) return 4;
        auto payload_byte = [&](std::size_t offset) { return at<std::uint8_t>(at<void*>(unit, 0x538), offset); };
        if (host.object_vcall_5c_is_kind(unit, 0xD)) return payload_byte(0x808) != 0 ? 0x15 : 0xC;
        if (host.object_vcall_5c_is_kind(unit, 0xA)) return 7 + (payload_byte(0x808) != 0 ? 1 : 0);
        if (host.object_vcall_5c_is_kind(unit, 8)) return 5;
        if (host.object_vcall_5c_is_kind(unit, 0xB)) return 9;
        if (host.object_vcall_5c_is_kind(unit, 0xC)) return payload_byte(0x81A) != 0 ? 0xA : 0xB;
        if (host.object_vcall_5c_is_kind(unit, 9)) return payload_byte(0x82C) != 0 ? 0x16 : 6;
        return 3;
    }
    void* kind = class_object;
    if (host.class_vcall_18_is_kind(kind, 0x12)) return 0xE;
    if (host.class_vcall_18_is_kind(kind, 0x13)) return 0xF;
    if (host.class_vcall_18_is_kind(kind, 0x10)) return 0x11;
    if (host.class_vcall_18_is_kind(kind, 0x11)) return 0x13;
    if (host.class_vcall_18_is_kind(kind, 0x16)) return 0x10;
    if (host.class_vcall_18_is_kind(kind, 0x15)) return 0x10;
    if (host.class_vcall_18_is_kind(kind, 0xE)) return 0xD;
    if (host.class_vcall_18_is_kind(kind, 7)) return 4;
    if (host.class_vcall_18_is_kind(kind, 0xD)) return at<std::uint8_t>(kind, 0x808) != 0 ? 0x15 : 0xC;
    if (host.class_vcall_18_is_kind(kind, 0xA)) return 7 + (at<std::uint8_t>(kind, 0x808) != 0 ? 1 : 0);
    if (host.class_vcall_18_is_kind(kind, 8)) return 5;
    if (host.class_vcall_18_is_kind(kind, 0xB)) return 9;
    if (host.class_vcall_18_is_kind(kind, 0xC)) return at<std::uint8_t>(kind, 0x81A) != 0 ? 0xA : 0xB;
    if (host.class_vcall_18_is_kind(kind, 9)) return at<std::uint8_t>(kind, 0x82C) != 0 ? 0x16 : 6;
    return 3;
}

// ---------------------------------------------------------------------------
// 00534870 BSP_Hud_UnitOrderIcon (the formation screen's selected-unit panel)
// __thiscall(ECX = screen), RET 0. No EH frame.
// unit = *0070D070(+138h, 00A9C920(+0ACh)); null -> return. icon = 0; when
// unit->+114h() (the director) is non-null and 0071BE40(director) names a
// class: 00E08F10/18/20/28/38/40/48/50/30/58/78 -> 3, 00E08F60/68/80 -> 1,
// 00E08F70 -> 2, 00E08F88/90 -> 0, 00E08FA0 -> 4, anything else 0 (the
// compare chain 005348BE-005349C3; attack_command_order_icon in
// src/attack_commands.cpp states the same table by command id).
// +0D0h->+88h(icon, 0, 1.0f); 00ABBE50(+0B8h, unit->+14h(), 1); unit+54h set
// -> +0C0h->+34h(0), +0BCh->+34h(1), else +0BCh->+34h(0), +0C0h->+34h(1).
// unit->+5Ch(5): +0C8h shown, +0C8h->+48h({(float)00923BE0(unit), 1.0f});
// then unit->+5Ch(6): a/b/c/d = _ftol2 of the getters +38h/+34h/+30h/+2Ch
// of unit+0A20h (called in that order), hp = unit+36Ch, +0CCh->+48h({(float)
// (00923BE0(unit) - (double)(d*b + c*a) / hp), 1.0f}) (integer products and
// sum, FILD then FDIV by the float hp, FSUBP ST1 = health - quotient); not
// kind 6: +0CCh->+48h({(float)00923BE0(unit), 1.0f}) then +0C8h->+48h({0.0f,
// 1.0f}). Always (also when not kind 5): +0C4h->+88h(00653200([00E198C4] +
// 0BCh, unit+538h), 0, 1.0f). No RNG draws.
void cc13_sweepB4_hud_unit_order_icon_00534870(void* screen, Cc13SweepB4Host& host) {
    const std::int32_t index = host.listbox_selected_index_00a9c920(at<void*>(screen, 0xAC));
    void* unit = *static_cast<void**>(host.unit_group_member_record_at_0070d070(at<void*>(screen, 0x138), index));
    if (unit == nullptr) {
        return;
    }
    std::int32_t icon = 0;
    void* director = host.unit_vcall_114_director(unit);
    if (director != nullptr) {
        void* command = host.director_current_command_0071be40(director);
        if (command != nullptr) {
            struct Arm {
                std::uint32_t address;
                std::int32_t icon;
            };
            static const Arm kArms[] = {
                {0x00E08F10, 3}, {0x00E08F18, 3}, {0x00E08F20, 3}, {0x00E08F28, 3}, {0x00E08F38, 3},
                {0x00E08F40, 3}, {0x00E08F48, 3}, {0x00E08F50, 3}, {0x00E08F30, 3}, {0x00E08F58, 3},
                {0x00E08F60, 1}, {0x00E08F68, 1}, {0x00E08F70, 2}, {0x00E08F78, 3}, {0x00E08F80, 1},
                {0x00E08F88, 0}, {0x00E08F90, 0}, {0x00E08FA0, 4},
            };
            for (const Arm& arm : kArms) {
                if (command == host.image_global(arm.address)) {
                    icon = arm.icon;
                    break;
                }
            }
        }
    }
    host.widget_vcall_88(at<void*>(screen, 0xD0), icon, 0, 1.0f);
    host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, 0xB8), host.object_vcall_14_name(unit), 1);
    if (at<std::int32_t>(unit, 0x54) != 0) {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xC0), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC), 1);
    } else {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xBC), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xC0), 1);
    }
    if (host.object_vcall_5c_is_kind(unit, 5)) {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xC8), 1);
        const float health[2] = {static_cast<float>(host.unit_health_00923be0(unit)), 1.0f};
        host.widget_vcall_48_set_pair(at<void*>(screen, 0xC8), health);
        if (host.object_vcall_5c_is_kind(unit, 6)) {
            void* motion = static_cast<std::uint8_t*>(unit) + 0xA20;
            const std::int32_t a = ftol_trunc(host.getter_38_00939f70(motion));
            const std::int32_t b = ftol_trunc(host.getter_34_00939f80(motion));
            const std::int32_t c = ftol_trunc(host.getter_30_00939fc0(motion));
            const std::int32_t d = ftol_trunc(host.getter_2c_00939fb0(motion));
            const float hp = at<float>(unit, 0x36C);
            const double now = host.unit_health_00923be0(unit);
            const std::int32_t sum = static_cast<std::int32_t>(static_cast<std::uint32_t>(d) * static_cast<std::uint32_t>(b) +
                                                               static_cast<std::uint32_t>(c) * static_cast<std::uint32_t>(a));
            const float bar[2] = {static_cast<float>(now - static_cast<double>(sum) / hp), 1.0f};
            host.widget_vcall_48_set_pair(at<void*>(screen, 0xCC), bar);
        } else {
            const float bar[2] = {static_cast<float>(host.unit_health_00923be0(unit)), 1.0f};
            host.widget_vcall_48_set_pair(at<void*>(screen, 0xCC), bar);
            const float empty[2] = {0.0f, 1.0f};
            host.widget_vcall_48_set_pair(at<void*>(screen, 0xC8), empty);
        }
    }
    void* gui = at<void*>(host.global_00e198c4(), 0xBC);
    const std::int32_t frame = host.hud_payload_icon_frame_00653200(gui, at<void*>(unit, 0x538));
    host.widget_vcall_88(at<void*>(screen, 0xC4), frame, 0, 1.0f);
}

}  // namespace bsp
