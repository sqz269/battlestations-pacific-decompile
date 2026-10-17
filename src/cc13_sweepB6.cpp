// cc13_sweepB6: sixth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB6, docs/CC13_SWEEPB6.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB..B5): float expressions follow the image's
// operation order with a float store wherever the image stores to a float
// slot; intermediate x87 values are kept in double (53-bit precision control),
// and double constants are applied in double. EH frames are noted, not
// modelled. Register-register x87 operand directions were read from the bytes
// (local/sweepB6_dis.py decodes the DC/DE forms by the Intel encoding).
//
// Checked-iterator note: the inlined STL code tests its iterators and calls
// 00BF6713 on failure. The tests on game data (indices against a vector's
// size, first <= last, an iterator against last) are reproduced; the tests
// that compare a local iterator's container pointer with the address of the
// local vector it was taken from cannot fail and are left out (each function
// names them).

#include "bsp/cc13_sweepB6.hpp"

#include <cmath>
#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

// A checked dword vector as the image lays it out on the stack and in objects:
// {proxy, first, last, end}. The image never writes the proxy word of its
// local vectors; it is zeroed here only because C++ needs a value.
struct DwordVector {
    void* proxy;
    std::uint32_t* first;
    std::uint32_t* last;
    std::uint32_t* end;
};

// Element count of the vector whose `first` word is at base+offset (the
// `last` word follows): 0 when first is null, else (last - first) >> 2.
std::uint32_t vector_count(void* base, std::size_t first_offset) {
    const std::uint32_t first = at<std::uint32_t>(base, first_offset);
    if (first == 0) {
        return 0;
    }
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(base, first_offset + 4) - first) >> 2);
}

// The inlined native-string build from a literal (as sweepB5): {0, null},
// 0041DD40(&s, length, 1), then when data is non-null 00BF7680(data,
// literal, length + 1) with the length the resize left.
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
// back to the sized pool (00419CC0, 00BD1510) with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepBHost& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The pointer stored in an image global (host.image_global gives its address).
void* image_pointer(Cc13SweepBHost& host, std::uint32_t address) {
    return *static_cast<void**>(host.image_global(address));
}

// Find the child named by a literal (built and released around 00AA7E00(row,
// &name, 1)); `use` runs on the child while the name is alive, as in the image.
template <class Use>
void with_child(void* row, std::uint32_t literal, std::uint32_t length, Cc13SweepB6Host& host, Use use) {
    Cc13SweepBNativeString name;
    build_literal(name, literal, length, host);
    use(host.widget_find_child_00aa7e00(row, &name, 1));
    release_string(name, host);
}

}  // namespace

namespace {

double f64_bits(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// Pushes a widget into a {data, size, cap} pointer array the inlined way:
// when size == cap, reserve(max(cap * 2, 1)) first (cmp; jg; signed); then
// when data + size is non-null store the widget there; size += 1 always.
template <class Reserve>
void ptr_array_push(void* array, void* widget, Reserve reserve) {
    const std::int32_t cap = at<std::int32_t>(array, 8);
    if (at<std::int32_t>(array, 4) == cap) {
        std::int32_t grown = cap + cap;
        if (!(grown > 1)) {
            grown = 1;
        }
        reserve(grown);
    }
    void** slot = at<void**>(array, 0) + at<std::int32_t>(array, 4);
    if (slot != nullptr) {
        *slot = widget;
    }
    at<std::int32_t>(array, 4) += 1;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00547A20 (weapon-group sight for one weapon kind; no ledger name)
// ABI: __thiscall(ECX = screen, [esp+4] int kind), RET 4. EH frame (handler
// 00C6CE70; state 1 covers the two local vectors; unwinding not modelled).
// Only caller 00548300 (kinds 5, 2, 7, 9 for groups 2..5). Frame after the
// four register pushes: [esp+10h] group offset, +14h the record array /
// x scratch, +18h the slot index ebx, +1Ch scratch, +20h group index /
// sin scratch, +2Ch yaw, +30h pitch, +3Ch..+44h the unit position, +48h and
// +54h two float[3] positions, vectors A {+70h, first +74h, last +78h,
// end +7Ch} and B {+60h, first +64h, last +68h, end +6Ch}.
//
// Returns at once when [screen+24h] (group count) is 0, when
// [[screen+20h]+14h] is null, or when the unit [screen+40h] is null. Then
// byte +0D5h = 0; the unit pose is refreshed (00414DB0 when byte +0C8h is
// clear) before the position (unit +0FCh/+100h/+104h) is read and twice more
// after it; 00521370(unit+0ECh, &pitch, &yaw); 00412E20(float(float(
// [[[00E188A8]+19FCh]+1C4h]) * 0.5)) runs and its ST0 is discarded (fstp st0).
// Byte +0E0h = 0; slot = 0.
// For each group g < [+24h] (stride 18h at [+20h]) with entry e =
// [group+14h] non-null: records = [e+538h]+94h; for gun = [e+48h] through
// [gun+44h]: the kind gate (jump table 005482B0 over kind-2): kinds 2..4 need
// vtable +5Ch(24h) and 005459B0; kind 5 vtable +5Ch(24h) and 005459E0; kind 7
// vtable +5Ch(24h) and [[gun+3F4h]+80h] == 7; kind 9 the same with 9, and
// then byte +0D5h = 1; every other kind skips the gun. A matched gun:
//  - when [+0E8h] == slot: a clone of [+0FCh] is pushed into {+0E4h} (reserve
//    00545CB0) and a clone of [+0D8h] into {+0F0h} (reserve 00545D10);
//  - state = 0. When !00727D30(gun, 0.0f) && 00729A40(gun) && 00729F10(gun):
//    (byte +1Ch clear) +0E4h[slot] +34h(1), then (re-tested) +0F0h[slot]
//    +34h(1); i = 00727C30(gun), t = [gun+418h][i]; when t != 0 (ucomiss;
//    lahf; test ah,44h; jnp, so NaN divides): r = float([gun+414h][i] / t),
//    00ABE6E0(+0F0h[slot], 1 - (r > 1 ? 1 : r), 0, 0, 1.0f) (fcomip; ja, so NaN
//    keeps r); state = 6.
//    Otherwise, with byte +1Ch set nothing more; else state = 00547480(e, gun,
//    kind, &position, pitch, yaw) (the cc13_sweepB5 reconstruction is called),
//    byte +0E0h = 1 when it was clear and state == 4, +0E4h[slot] +34h(1);
//    then when [gun+450h] > now - [gun+474h] (fcomip; jbe, NaN hides):
//    +0F0h[slot] +34h(1) and, with g = [gun+450h] (comiss g, 0.5; jb), g < 0.5
//    or NaN: 00ABE6E0(.., 1, 0, 0, 1); else 00ABE6E0(.., float((now -
//    [gun+474h]) / g), 0, 0, 1) and state = 6 (the image joins the barrel path
//    at 00547DD1); not above: +0F0h[slot] +34h(0).
//  - +0E4h[slot] vtable +88h(state, 0, 1.0f); idx = [gun+38Ch]; when idx >=
//    [records+4] 005471B0(records, idx + 1); the slot index is pushed
//    (00442190) into A when byte [[records]+idx*4]+0Eh is set, else into B;
//    slot += 1.
// Unused slots slot..[+0E8h]-1: +0E4h and +0F0h entries get +34h(0).
// Arcs, for n = |A| (then |B|): half = (double)(n - 1) * 0.5; for i < n:
// a = float(float(i - half) * 0.20943951606750488); x = float(float(sin a) *
// 0.063 + [+48h]); c = float(cos a). A: y = float([+4Ch] - c * 0.084) (fsubp
// ST1-ST0); +0E4h[A[i]] gets 00AA7DC0({x, y, 0}) and +0F0h[A[i]] gets {x, y,
// -0.1f}. B: y = float(c * 0.084 + [+4Ch]) with the same two calls on B[i].
// The 0.2094.., 0.063 and 0.084 are float-valued doubles (00CE3E00,
// 00CEDF68, 00CEDF60); fsin/fcos are taken as std::sin/std::cos of the float
// angle (a last-ulp difference before the float store is possible). Index
// reads of A and B are checked (00BF6713). Last: 00BF65AC(B.first) and
// 00BF65AC(A.first) when non-null. No RNG draws.
void cc13_sweepB6_weapon_group_sight_00547a20(void* screen, std::int32_t kind, Cc13SweepB6Host& host) {
    auto* const base = static_cast<std::uint8_t*>(screen);
    if (at<std::int32_t>(screen, 0x24) == 0) {
        return;
    }
    if (at<void*>(at<void*>(screen, 0x20), 0x14) == nullptr) {
        return;
    }
    void* unit = at<void*>(screen, 0x40);
    if (unit == nullptr) {
        return;
    }
    at<std::uint8_t>(screen, 0xD5) = 0;
    if (at<std::uint8_t>(unit, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
    }
    const float position[3] = {at<float>(unit, 0xFC), at<float>(unit, 0x100), at<float>(unit, 0x104)};
    if (at<std::uint8_t>(at<void*>(screen, 0x40), 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(at<void*>(screen, 0x40));
    }
    unit = at<void*>(screen, 0x40);
    if (at<std::uint8_t>(unit, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
    }
    float pitch = 0.0f;
    float yaw = 0.0f;
    host.direction_to_pitch_yaw_00521370(&at<float>(unit, 0xEC), &pitch, &yaw);
    {
        void* view = at<void*>(image_pointer(host, 0x00E188A8), 0x19FC);
        const float fov = at<float>(view, 0x1C4);
        const float half = static_cast<float>(static_cast<double>(fov) * 0.5);
        static_cast<void>(host.math_tangent_00412e20(half));  // the image discards ST0 (fstp st0)
    }
    at<std::uint8_t>(screen, 0xE0) = 0;
    std::int32_t slot = 0;
    DwordVector list_a{};
    DwordVector list_b{};

    for (std::int32_t group = 0; group < at<std::int32_t>(screen, 0x24); ++group) {
        void* entry = at<void*>(at<std::uint8_t*>(screen, 0x20) + group * 0x18, 0x14);
        if (entry == nullptr) {
            continue;
        }
        void* records = at<std::uint8_t*>(entry, 0x538) + 0x94;
        for (void* gun = at<void*>(entry, 0x48); gun != nullptr; gun = at<void*>(gun, 0x44)) {
            bool matched = false;
            switch (kind) {
            case 2:
            case 3:
            case 4:
                matched = host.entity_vcall_5c_is_kind(gun, 0x24) && host.gun_is_artillery_kind_005459b0(gun);
                break;
            case 5:
                matched = host.entity_vcall_5c_is_kind(gun, 0x24) && host.gun_is_torpedo_class_launcher_005459e0(gun);
                break;
            case 7:
                matched = host.entity_vcall_5c_is_kind(gun, 0x24) &&
                          at<std::int32_t>(at<void*>(gun, 0x3F4), 0x80) == 7;
                break;
            case 9:
                if (host.entity_vcall_5c_is_kind(gun, 0x24) && at<std::int32_t>(at<void*>(gun, 0x3F4), 0x80) == 9) {
                    at<std::uint8_t>(screen, 0xD5) = 1;
                    matched = true;
                }
                break;
            default:
                break;
            }
            if (!matched) {
                continue;
            }
            if (at<std::int32_t>(screen, 0xE8) == slot) {
                void* marker = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xFC), nullptr);
                ptr_array_push(base + 0xE4, marker,
                               [&](std::int32_t n) { host.ptr_array_reserve_00545cb0(base + 0xE4, n); });
                void* bar = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xD8), nullptr);
                ptr_array_push(base + 0xF0, bar,
                               [&](std::int32_t n) { host.ptr_array_reserve_00545d10(base + 0xF0, n); });
            }
            auto marker_at = [&] { return at<void**>(screen, 0xE4)[slot]; };
            auto bar_at = [&] { return at<void**>(screen, 0xF0)[slot]; };
            std::int32_t state = 0;
            if (!host.gun_any_barrel_ready_within_00727d30(gun, 0.0f) &&
                host.gun_any_barrel_timer_below_threshold_00729a40(gun) && host.gun_is_operational_00729f10(gun)) {
                if (at<std::uint8_t>(screen, 0x1C) == 0) {
                    host.widget_vcall_34_set_visible(marker_at(), 1);
                    if (at<std::uint8_t>(screen, 0x1C) == 0) {
                        host.widget_vcall_34_set_visible(bar_at(), 1);
                    }
                }
                const std::int32_t barrel = host.gun_index_of_readiest_barrel_00727c30(gun);
                const float t = at<float*>(gun, 0x418)[barrel];
                if (!(t == 0.0f)) {
                    const float r = static_cast<float>(static_cast<double>(at<float*>(gun, 0x414)[barrel]) / t);
                    const float m = r > 1.0f ? 1.0f : r;
                    host.gui_widget_set_quad_floats_00abe6e0(bar_at(), static_cast<float>(1.0 - m), 0.0f, 0.0f,
                                                             1.0f);
                }
                state = 6;
            } else if (at<std::uint8_t>(screen, 0x1C) == 0) {
                state = cc13_sweepB5_weapon_group_gun_sight_status_00547480(screen, entry, gun, kind, position, pitch,
                                                                            yaw, host);
                if (at<std::uint8_t>(screen, 0xE0) == 0 && state == 4) {
                    at<std::uint8_t>(screen, 0xE0) = 1;
                }
                host.widget_vcall_34_set_visible(marker_at(), 1);
                const double elapsed = static_cast<double>(host.global_00f876a4()) - at<float>(gun, 0x474);
                if (static_cast<double>(at<float>(gun, 0x450)) > elapsed) {
                    host.widget_vcall_34_set_visible(bar_at(), 1);
                    const float g = at<float>(gun, 0x450);
                    if (g < 0.5f || g != g) {
                        host.gui_widget_set_quad_floats_00abe6e0(bar_at(), 1.0f, 0.0f, 0.0f, 1.0f);
                    } else {
                        const double since = static_cast<double>(host.global_00f876a4()) - at<float>(gun, 0x474);
                        host.gui_widget_set_quad_floats_00abe6e0(bar_at(), static_cast<float>(since / g), 0.0f, 0.0f,
                                                                 1.0f);
                        state = 6;
                    }
                } else {
                    host.widget_vcall_34_set_visible(bar_at(), 0);
                }
            }
            host.widget_vcall_88(marker_at(), state, 0, 1.0f);
            const std::int32_t index = at<std::int32_t>(gun, 0x38C);
            if (index >= at<std::int32_t>(records, 4)) {
                host.ref_pointer_array_resize_005471b0(records, index + 1);
            }
            void* record = at<void**>(records, 0)[index];
            const std::int32_t value = slot;
            if (at<std::uint8_t>(record, 0x0E) != 0) {
                host.dword_vector_push_back_00442190(&list_a, &value);
            } else {
                host.dword_vector_push_back_00442190(&list_b, &value);
            }
            ++slot;
        }
    }
    for (std::int32_t i = slot; i < at<std::int32_t>(screen, 0xE8); ++i) {
        host.widget_vcall_34_set_visible(at<void**>(screen, 0xE4)[i], 0);
        host.widget_vcall_34_set_visible(at<void**>(screen, 0xF0)[i], 0);
    }

    // The two arcs; `upper` selects A's y (minus) and placement order.
    auto arc = [&](DwordVector& list, bool upper) {
        const std::int32_t n = list.first == nullptr ? 0 : static_cast<std::int32_t>(list.last - list.first);
        if (n <= 0) {
            return;
        }
        const double half = static_cast<double>(n - 1) * 0.5;
        for (std::int32_t i = 0; i < n; ++i) {
            const float t = static_cast<float>(static_cast<double>(i) - half);
            const float a = static_cast<float>(static_cast<double>(t) * f64_bits(0x3FCACEEA00000000ull));
            const float s = static_cast<float>(std::sin(static_cast<double>(a)));
            const float x = static_cast<float>(static_cast<double>(s) * f64_bits(0x3FB020C480000000ull) +
                                               at<float>(screen, 0x48));
            const float c = static_cast<float>(std::cos(static_cast<double>(a)));
            const double cy = static_cast<double>(c) * f64_bits(0x3FB5810600000000ull);
            const float y = upper ? static_cast<float>(static_cast<double>(at<float>(screen, 0x4C)) - cy)
                                  : static_cast<float>(cy + at<float>(screen, 0x4C));
            auto checked = [&] {
                if (list.first == nullptr ||
                    static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>(list.last - list.first)) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                return list.first[i];
            };
            const float marker_pos[3] = {x, y, 0.0f};
            host.gui_widget_set_local_position_00aa7dc0(at<void**>(screen, 0xE4)[checked()], marker_pos);
            const float bar_pos[3] = {x, y, cc13_sweepB_f32(0xBDCCCCCDu)};  // 00CE3CB4
            host.gui_widget_set_local_position_00aa7dc0(at<void**>(screen, 0xF0)[checked()], bar_pos);
        }
    };
    arc(list_a, true);
    arc(list_b, false);
    if (list_b.first != nullptr) {
        host.crt_operator_delete_00bf65ac(list_b.first);
    }
    if (list_a.first != nullptr) {
        host.crt_operator_delete_00bf65ac(list_a.first);
    }
}

// ---------------------------------------------------------------------------
// 005A7C20 BSP_HudMapIcon_RefreshPowerUpIcons
// ABI: __thiscall(ECX = icon, [esp+4] template widget, [esp+8] char second),
// RET 8, EAX = the count. EH frame (handler 00C719C8; state 0 covers the local
// vector, which the image frees on both exits; unwinding is not modelled).
// Frame (after the four register pushes): [esp+10h] index / clone slot,
// [esp+14h] icon, [esp+18h] the iterator's container, [esp+30h] the local
// vector {proxy, first +34h, last +38h, end +3Ch}; the arguments at +50h/+54h.
//
// The list is {proxy +88h, first +8Ch, last +90h, end +94h} with the stamp at
// +44h when `second`, else {+0C8h, +0CCh, +0D0h, +0D4h} with the stamp at +40h.
// Rate gate (005A7C50): fld [stamp]; fadd qword [00D7A210] (1.0); fld
// [00F876A4]; fcomip st(1); ja -> taken only when now > stamp + 1 (ordered);
// NaN takes the not-above arm. Not above: with a template the function returns
// the list's count at once; without one it carries on. Above: with a template
// the stamp is set to now (movss, float store).
//
// Collection (unit = [icon+4], skipped when null): 008E9AF0 on [00F88C30] with
// icon+190h, +150h, +160h, +170h, +180h, 1, 1. Pass 1 walks the vector
// +190h (first +194h); pass 2 walks +150h (first +154h) and keeps an entry
// only when [+184h][i] - now > 0 (fld; fsub [00F876A4]; fldz; fxch; fcomip;
// jbe skips, so NaN skips). Each entry that 008E62A0 accepts for the unit
// (re-read from icon+4 at each call) is added to the local vector unless it is
// already there (an inlined find, then an inlined push_back with 005A6580 as
// the slow path).
// Without a template: the local vector is freed (00BF65AC) when non-null and
// its count is returned (0 when it was never allocated).
// With a template: for each collected entry 00AAB4C0(template, 0) clones a
// widget; a non-null clone gets vtable +34h(0), the icon state of row
// `kind` = [[icon+4]+54h], replaced by [[[00E188A8]+18CCh+[[00E188A8]+18ECh]*4]+28h]
// when kind >= 2 (signed jl): 00AB64C0(clone, [[entry+4]+30h+kind*38h],
// [entry+4]+34h+kind*38h), then vtable +88h(1, 0, 1.0f), then an inlined
// push_back into the list with 0043DB60 as the slow path. Then the local
// vector is freed when non-null and the list's count is returned.
//
// Left out: the container-pointer tests at 005A7EAF/5A7EC4, 005A807F/5A8094,
// 005A81BF and 005A8224/5A833F (the iterator's container is always the local
// vector). The erase after the release loop moves (last - last) elements, so
// its memmove_s (00BF67A7) only runs for a count the image computes as > 0;
// the computation is kept as written.
std::int32_t cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(void* icon, void* template_widget,
                                                                     bool second, Cc13SweepB6Host& host) {
    const std::size_t list = second ? 0x88 : 0xC8;
    float& stamp = at<float>(icon, second ? 0x44 : 0x40);
    const double gate = static_cast<double>(stamp) + 1.0;
    if (static_cast<double>(host.global_00f876a4()) > gate) {
        if (template_widget != nullptr) {
            stamp = host.global_00f876a4();
        }
    } else if (template_widget != nullptr) {
        return static_cast<std::int32_t>(vector_count(icon, list + 4));
    }

    // Release every widget in the list (vtable +4h(1)) and null its slot.
    std::uint32_t first_seen = 0;
    for (std::uint32_t i = 0;; ++i) {
        first_seen = at<std::uint32_t>(icon, list + 4);
        if (first_seen == 0 ||
            i >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(icon, list + 8) - first_seen) >> 2)) {
            break;
        }
        void** slot = at<void**>(icon, list + 4) + i;
        if (*slot != nullptr) {
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
    }
    // erase(begin, end), inlined.
    {
        const std::uint32_t erase_end = at<std::uint32_t>(icon, list + 8);
        if (first_seen > erase_end) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        const std::uint32_t first = at<std::uint32_t>(icon, list + 4);
        if (first > at<std::uint32_t>(icon, list + 8)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (first != erase_end) {
            const std::int32_t count = static_cast<std::int32_t>(at<std::uint32_t>(icon, list + 8) - erase_end) >> 2;
            const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4u;
            const std::uint32_t new_last = first + bytes;
            if (count > 0) {
                host.crt_memmove_s_00bf67a7(reinterpret_cast<void*>(first), bytes,
                                            reinterpret_cast<const void*>(erase_end), bytes);
            }
            at<std::uint32_t>(icon, list + 8) = new_last;
        }
    }

    DwordVector found{};
    found.first = nullptr;
    found.last = nullptr;
    found.end = nullptr;

    // Adds the entry at `source` to `found` unless an equal dword is there.
    // `entries` is the offset of the walked vector's first word.
    auto add_unique = [&](std::size_t entries, std::uint32_t index) {
        auto check_index = [&] {
            const std::uint32_t f = at<std::uint32_t>(icon, entries);
            if (f == 0 ||
                index >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(icon, entries + 4) - f) >> 2)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
        };
        std::uint32_t* it = found.first;
        if (found.first > found.last) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        for (;;) {
            if (found.first > found.last) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (it == found.last) {
                break;
            }
            if (it >= found.last) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            check_index();
            if (*it == at<std::uint32_t*>(icon, entries)[index]) {
                return;
            }
            if (it >= found.last) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            ++it;
        }
        check_index();
        const std::uint32_t* source = at<std::uint32_t*>(icon, entries) + index;
        if (found.first != nullptr && static_cast<std::uint32_t>(found.last - found.first) <
                                          static_cast<std::uint32_t>(found.end - found.first)) {
            *found.last = *source;
            ++found.last;
            return;
        }
        if (found.first > found.last) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        std::uint32_t out[2];
        host.dword_vector_insert_one_005a6580(&found, out, &found, found.last, source);
    };

    if (at<void*>(icon, 4) != nullptr) {
        auto* const base = static_cast<std::uint8_t*>(icon);
        host.powerup_manager_collect_active_008e9af0(host.global_00f88c30(), base + 0x190, base + 0x150, base + 0x160,
                                                     base + 0x170, base + 0x180, 1, 1);
        for (std::uint32_t i = 0;; ++i) {
            if (vector_count(icon, 0x194) <= i) {
                break;
            }
            const std::uint32_t f = at<std::uint32_t>(icon, 0x194);
            if (f == 0 || i >= vector_count(icon, 0x194)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* entry = at<void**>(icon, 0x194)[i];
            if (host.powerup_applies_to_unit_008e62a0(entry, at<void*>(icon, 4))) {
                add_unique(0x194, i);
            }
        }
        for (std::uint32_t i = 0;; ++i) {
            if (vector_count(icon, 0x154) <= i) {
                break;
            }
            if (at<std::uint32_t>(icon, 0x184) == 0 || i >= vector_count(icon, 0x184)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            const double left = static_cast<double>(at<float*>(icon, 0x184)[i]) -
                                static_cast<double>(host.global_00f876a4());
            if (!(left > 0.0)) {
                continue;
            }
            if (at<std::uint32_t>(icon, 0x154) == 0 || i >= vector_count(icon, 0x154)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* entry = at<void**>(icon, 0x154)[i];
            if (host.powerup_applies_to_unit_008e62a0(entry, at<void*>(icon, 4))) {
                add_unique(0x154, i);
            }
        }
    }

    if (template_widget == nullptr) {
        std::int32_t count = 0;
        if (found.first != nullptr) {
            count = static_cast<std::int32_t>(found.last - found.first);
            host.crt_operator_delete_00bf65ac(found.first);
        }
        return count;
    }

    if (found.first > found.last) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    for (std::uint32_t* it = found.first;;) {
        if (found.first > found.last) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (it == found.last) {
            break;
        }
        void* clone = host.widget_clone_subtree_00aab4c0(template_widget, nullptr);
        if (clone != nullptr) {
            host.widget_vcall_34_set_visible(clone, 0);
            std::int32_t kind = at<std::int32_t>(at<void*>(icon, 4), 0x54);
            if (kind >= 2) {
                void* root = host.global_00e188a8();
                void* row = at<void*>(root, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(root, 0x18EC)) * 4);
                kind = at<std::int32_t>(row, 0x28);
            }
            if (it >= found.last) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            if (it >= found.last) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            auto* const record = at<std::uint8_t*>(reinterpret_cast<void*>(*it), 4);
            const std::size_t row_offset = static_cast<std::size_t>(kind) * 0x38;
            host.gui_icon_add_state_00ab64c0(clone, at<void*>(record, row_offset + 0x30), record + row_offset + 0x34);
            host.widget_vcall_88(clone, 1, 0, 1.0f);
            const std::uint32_t lf = at<std::uint32_t>(icon, list + 4);
            const std::uint32_t size = lf == 0 ? 0u
                                               : static_cast<std::uint32_t>(
                                                     static_cast<std::int32_t>(at<std::uint32_t>(icon, list + 8) - lf) >> 2);
            if (lf != 0 &&
                size < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(icon, list + 12) - lf) >> 2)) {
                void** last = at<void**>(icon, list + 8);
                *last = clone;
                at<void**>(icon, list + 8) = last + 1;
            } else {
                const std::uint32_t where = at<std::uint32_t>(icon, list + 8);
                if (lf > where) {
                    host.crt_invalid_parameter_noinfo_00bf6713();
                }
                std::uint32_t out[2];
                void* vector = static_cast<std::uint8_t*>(icon) + list;
                host.dword_vector_insert_one_0043db60(vector, out, vector, reinterpret_cast<void*>(where), &clone);
            }
        }
        if (it >= found.last) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        ++it;
    }
    if (found.first != nullptr) {
        host.crt_operator_delete_00bf65ac(found.first);
    }
    return static_cast<std::int32_t>(vector_count(icon, list + 4));
}

// ---------------------------------------------------------------------------
// 005A8600 BSP_HudMapScreen_Destruct
// ABI: __thiscall(ECX = screen), plain RET; EBP frame aligned to 8, EH frame
// (handler 00C71B29, states 10h..0 as the members go, plus a flags word
// [esp+1Ch] for the two conditional name strings; unwinding not modelled).
// Only caller 005AB140 (the GUI_map scalar deleting destructor).
//
// Vtables: +0h = 00CF0134, +8h = 00CF0130, +64h = 00CF0118.
// Terrain (only when +56Ch is set), as the exit 005A4200 but simpler: for
// each child of [[+0Ch]+0Ch] (+3Ch next) whose name (00B6D800 +4h) is
// non-null and 00BF7FBF-equal to "Terrain Visibility Group" (00CF00D8): for
// each of its children (+34h first, +3Ch next) "BushGroup" (00CF00CC, 9) is
// built (flag 1) and compared by the inlined native-string == (empty name ->
// equal iff the literal is empty; empty literal -> unequal; else 00BF7FBF ==
// 0); when unequal "TreeGroup" (00CF00C0, 9) is built (flag 2) and compared;
// TreeGroup then BushGroup are released; a child equal to neither gets
// 00B6DA70(child, 1.0f, 0). The group is then appended to {+570h, +574h,
// +578h} (0059E5E0(max(2 * cap, 1)) when full; a null slot address is not
// written). Then 00B6D890(+570h[i], +56Ch) for i < count (signed) and
// 0059FCE0(+570h, 0).
// Then byte [[00E188A8]+19C4h] = 0; 00AA31F0(004C12B0(), +8Ch) and the same
// for +90h. The list +498h (head +49Ch): every node value v (+8h, no null
// test) gets InterlockedDecrement(v+4) (IAT 00CE2220) and vtable +0h at zero
// (the end-dereference checks are kept; the container test is a cmp ebx,ebx
// and is left out).
// Ref-pointer slots (when non-null: InterlockedDecrement(p+4), vtable +0h at
// zero, slot = 0): for k < 61h: +130h+4k then +2B4h+4k; +438h..+44Ch;
// +450h..+460h; +464h; for k < 3: +468h+4k, +474h+4k, +480h+4k, +48Ch+4k;
// +4A4h..+4ACh; +108h..+11Ch; +5A0h, +5A4h, +5A8h. Then +128h = 0.
// Members: tree +600h (005A6350(tree, &out, tree, [head], tree, head),
// 00BF65AC(head), +604h = +608h = 0); vector +580h (00BF65AC(first) when
// set; three words cleared); the pointer array +570h (capacity < 0 -> a
// one-slot block from 00BF55BE(4) takes the count elements, the old data goes
// to 00BF6989 and capacity = 1; a negative count has its slots data[count..-1]
// zeroed where the address is non-null; the count is stepped down to 0 while
// positive, then 00BF6989(data) and count = 0); 00BF6989(+544h), +544h = 0;
// vectors +514h and +504h; tree +4F4h (00538360, head freed, +4F8h = +4FCh =
// 0); vectors +4E0h, +4CCh, +4B8h; the list +498h (head unlinked, size +4A0h
// = 0, nodes freed with 00BF65AC, then the head, +49Ch = 0); vectors +0F4h
// and +0D8h; tree +78h (005A5E40, head freed, +7Ch = +80h = 0); 00695870(
// +64h); 0093CC80(+8h); 004F71A0(screen). No RNG draws.
void cc13_sweepB6_hud_map_screen_destruct_005a8600(void* screen, Cc13SweepB6Host& host) {
    auto* const self = static_cast<std::uint8_t*>(screen);
    at<std::uint32_t>(screen, 0x00) = 0x00CF0134u;  // the image's vtable addresses, kept as values
    at<std::uint32_t>(screen, 0x08) = 0x00CF0130u;
    at<std::uint32_t>(screen, 0x64) = 0x00CF0118u;

    if (at<void*>(screen, 0x56C) != nullptr) {
        auto name_of = [&](void* node) {
            return static_cast<const Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(node));
        };
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
        for (void* group = at<void*>(at<void*>(screen, 0xC), 0xC); group != nullptr; group = at<void*>(group, 0x3C)) {
            const char* group_name = name_of(group)->data;
            if (group_name == nullptr ||
                host.crt_stricmp_00bf7fbf(group_name, static_cast<const char*>(host.image_global(0x00CF00D8u))) != 0) {
                continue;
            }
            for (void* child = at<void*>(group, 0x34); child != nullptr; child = at<void*>(child, 0x3C)) {
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
                if (neither) {
                    host.scene_node_set_visibility_factor_00b6da70(child, 1.0f, 0);
                }
            }
            void* array = self + 0x570;
            ptr_array_push(array, group, [&](std::int32_t n) { host.pointer_array_reserve_0059e5e0(array, n); });
        }
        for (std::int32_t i = 0; i < at<std::int32_t>(screen, 0x574); ++i) {
            host.node_propagate_root_00b6d890(at<void**>(screen, 0x570)[i], at<void*>(screen, 0x56C));
        }
        host.pointer_array_resize_0059fce0(self + 0x570, 0);
    }

    at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x19C4) = 0;
    host.gui_manager_remove_page_00aa31f0(host.gui_manager_get_004c12b0(), at<void*>(screen, 0x8C));
    host.gui_manager_remove_page_00aa31f0(host.gui_manager_get_004c12b0(), at<void*>(screen, 0x90));

    auto release = [&](void* object) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    {
        void* head = at<void*>(screen, 0x49C);
        void* node = at<void*>(head, 0);
        for (;;) {
            if (node == at<void*>(screen, 0x49C)) {
                break;
            }
            if (node == at<void*>(screen, 0x49C)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            release(at<void*>(node, 8));
            if (node == at<void*>(screen, 0x49C)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            node = at<void*>(node, 0);
        }
    }
    auto release_slot = [&](std::size_t offset) {
        void* object = at<void*>(screen, offset);
        if (object != nullptr) {
            release(object);
            at<void*>(screen, offset) = nullptr;
        }
    };
    for (std::size_t k = 0; k < 0x61; ++k) {
        release_slot(0x130 + 4 * k);
        release_slot(0x2B4 + 4 * k);
    }
    for (std::size_t k = 0; k < 6; ++k) {
        release_slot(0x438 + 4 * k);
    }
    for (std::size_t k = 0; k < 5; ++k) {
        release_slot(0x450 + 4 * k);
    }
    release_slot(0x464);
    for (std::size_t k = 0; k < 3; ++k) {
        release_slot(0x468 + 4 * k);
        release_slot(0x474 + 4 * k);
        release_slot(0x480 + 4 * k);
        release_slot(0x48C + 4 * k);
    }
    for (std::size_t k = 0; k < 3; ++k) {
        release_slot(0x4A4 + 4 * k);
    }
    for (std::size_t k = 0; k < 6; ++k) {
        release_slot(0x108 + 4 * k);
    }
    release_slot(0x5A0);
    release_slot(0x5A4);
    release_slot(0x5A8);
    at<std::uint32_t>(screen, 0x128) = 0;

    auto destroy_tree = [&](std::size_t offset, auto erase) {
        void* tree = self + offset;
        void* head = at<void*>(tree, 4);
        std::uint32_t out[2];
        erase(tree, out, tree, at<void*>(head, 0), tree, head);
        host.crt_operator_delete_00bf65ac(at<void*>(tree, 4));
        at<void*>(tree, 4) = nullptr;
        at<std::uint32_t>(tree, 8) = 0;
    };
    auto destroy_vector = [&](std::size_t first_offset) {
        void* first = at<void*>(screen, first_offset);
        if (first != nullptr) {
            host.crt_operator_delete_00bf65ac(first);
        }
        at<std::uint32_t>(screen, first_offset) = 0;
        at<std::uint32_t>(screen, first_offset + 4) = 0;
        at<std::uint32_t>(screen, first_offset + 8) = 0;
    };

    destroy_tree(0x600, [&](void* t, void* o, void* c1, void* f, void* c2, void* l) {
        host.tree_erase_range_005a6350(t, o, c1, f, c2, l);
    });
    destroy_vector(0x580);
    {
        // The pointer array +570h {data, count +574h, capacity +578h}, inlined.
        const std::int32_t capacity = at<std::int32_t>(screen, 0x578);
        if (capacity < 0 && capacity < 1) {
            auto* fresh = static_cast<void**>(host.crt_operator_new_00bf55be(4));
            void** dst = fresh;
            for (std::int32_t i = 0; i < at<std::int32_t>(screen, 0x574); ++i, ++dst) {
                if (dst != nullptr) {
                    *dst = at<void**>(screen, 0x570)[i];
                }
            }
            host.crt_free_00bf6989(at<void*>(screen, 0x570));
            at<void**>(screen, 0x570) = fresh;
            at<std::int32_t>(screen, 0x578) = 1;
        }
        const std::int32_t count = at<std::int32_t>(screen, 0x574);
        if (count < 0) {
            for (std::int32_t offset = count * 4; offset < 0; offset += 4) {
                auto* slot = reinterpret_cast<void**>(at<std::uint8_t*>(screen, 0x570) + offset);
                if (slot != nullptr) {
                    *slot = nullptr;
                }
            }
        }
        while (at<std::int32_t>(screen, 0x574) > 0) {
            at<std::int32_t>(screen, 0x574) += -1;
        }
        at<std::int32_t>(screen, 0x574) = 0;
        host.crt_free_00bf6989(at<void*>(screen, 0x570));
    }
    host.crt_free_00bf6989(at<void*>(screen, 0x544));
    at<std::uint32_t>(screen, 0x544) = 0;
    destroy_vector(0x514);
    destroy_vector(0x504);
    destroy_tree(0x4F4, [&](void* t, void* o, void* c1, void* f, void* c2, void* l) {
        host.tree_erase_range_00538360(t, o, c1, f, c2, l);
    });
    destroy_vector(0x4E0);
    destroy_vector(0x4CC);
    destroy_vector(0x4B8);
    {
        void* head = at<void*>(screen, 0x49C);
        void* node = at<void*>(head, 0);
        at<void*>(head, 0) = head;
        at<void*>(at<void*>(screen, 0x49C), 4) = at<void*>(screen, 0x49C);
        at<std::uint32_t>(screen, 0x4A0) = 0;
        while (node != at<void*>(screen, 0x49C)) {
            void* next = at<void*>(node, 0);
            host.crt_operator_delete_00bf65ac(node);
            node = next;
        }
        host.crt_operator_delete_00bf65ac(at<void*>(screen, 0x49C));
        at<void*>(screen, 0x49C) = nullptr;
    }
    destroy_vector(0xF4);
    destroy_vector(0xD8);
    destroy_tree(0x78, [&](void* t, void* o, void* c1, void* f, void* c2, void* l) {
        host.tree_erase_range_005a5e40(t, o, c1, f, c2, l);
    });
    host.callback_owner_destroy_00695870(self + 0x64);
    host.simple_3d_scene_destruct_0093cc80(self + 8);
    host.frontend_screen_destruct_004f71a0(screen);
}

// ---------------------------------------------------------------------------
// 005F24D0 BSP_OptionsScreen_BuildSettingsPage
// ABI: __thiscall(ECX = screen, [esp+4] const native string* title, [esp+8]
// labels (EDX of 005F0220), [esp+0Ch] label_count, [esp+10h] value_rows,
// [esp+14h] const int* row_kinds, [esp+18h] char list_pages), RET 18h. EH
// frame (handler 00C75DD8, states 0..0Ah over the local strings; unwinding
// not modelled). Callers 005F40F0, 005F41E0, 005F58F0, 005F59F0.
//
// 005F24EC..005F253E: 00A9AC40(+84h, 0), 00A9BEC0(+84h), 00A9CF30(+84h, 0, 1),
// 00A9AC40(+88h, 0), 00A9BEC0(+88h), 00549620([00E1930C], 1), 005189B0(ECX =
// title). screen+0ACh = value_rows.
// Value rows, i = 0 .. value_rows-1 (do-while after a jle guard): template
// by row_kinds[i]: 1 -> +9Ch, 2 -> +98h, 3 -> +94h, else +90h; row =
// 00AAB4C0(template, 0); 00AA6BC0(row, screen+0Ch, 0); row+0D8h = 1. Kinds 1
// and 3: the "arrow_left_Icon" (00CEB278, 0Fh) and "arrow_right_Icon"
// (00CEB264, 10h) children get 00AA6BC0(child, screen+0Ch, 0). Then the name
// "main_Text" (00CEE4CC, 9) is built, 005F0220(&label, labels, i) runs, then
// 00ABAED0(00AA7E00(row, &name, 1), label, 1); label then name released. When
// i != 0, row_kinds is the image table 00CF385C and byte screen+188h is clear:
// byte row+77h = 1 and 00AA6BC0(row, 0, 0). Then 00A9D750(+88h, row, 0, 0) and
// row vtable +34h(1).
// Page rows (list_pages only), i = value_rows .. label_count-1 (signed jge
// guard): row = 00AAB4C0(+8Ch, 0); 00ABAED0(row, 005F0220(&label, labels, i),
// 1); label released; 00A9D750(+84h, row, 0, 0); row vtable +34h(1).
// When [00E198C4] is non-null: mode [00E19650] == 1 -> 005F1620(screen,
// 00A9BE00(+88h, 0)); mode 3 -> the same for rows 0, 1 and 5.
// list_pages: 00A9AC40(+84h, screen+8), 00A9C7C0(+84h, [00E19630 +
// [screen+0A8h]*4]), +84h vtable +80h(1).
// Always: 00A9AC40(+88h, screen+8), +88h vtable +34h(1), byte [+88h]+14Ah = 0.
// list_pages: 00A9CDF0(ECX = +84h, EDX = +84h, +88h); strings "" (00CE3A0C)
// at +30h, "globals.back" (00CEB3A8, 0Ch) at +28h, "" at +10h, "" at +18h,
// "globals.select" (00CEB3EC, 0Eh) at +20h, built in that order; 0054B530(
// [00E1930C], (-5Eh, select, 0), (0, +18h, 1), (0, +10h, 1), (-5Dh, back, 2),
// (0, +30h, 1)); released +20h, +18h, +10h, +28h, +30h; +254h and +258h
// vtable +34h(0).
// Otherwise (as sweepB5's 005F3B50 tail): 00A9CDC0(+84h), +88h vtable +60h(1),
// 00A9C8F0(+88h, 0), [+88h]+12Ch/+130h/+134h = 0.003125f (00CEE944) /
// 0x3B360B61 (00CEE600) / 0, 005F0CD0(screen), 0054A0C0([00E1930C], &""),
// +254h vtable +34h(1), +254h vtable +84h([[00E188A8]+61Fh] != 0), +258h
// vtable +34h(movzx byte [[00E188A8]+61Fh]), then the list box's current row
// (null at the end iterator; checked-iterator tests kept) goes to the listener
// screen+8 vtable +8h(row, +88h).
// Last: byte +251h set -> clear it; else clear byte +250h. No RNG draws.
void cc13_sweepB6_options_screen_build_settings_page_005f24d0(void* screen, const void* title, const void* labels,
                                                              std::int32_t label_count, std::int32_t value_rows,
                                                              const std::int32_t* row_kinds, bool list_pages,
                                                              Cc13SweepB6Host& host) {
    auto* const self = static_cast<std::uint8_t*>(screen);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x84), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x84));
    host.gui_listbox_reset_highlight_00a9cf30(at<void*>(screen, 0x84), 0, 1);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x88), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x88));
    host.main_menu_command_owner_set_help_variant_00549620(image_pointer(host, 0x00E1930C), 1);
    host.front_end_frame_set_title_005189b0(title);
    at<std::int32_t>(screen, 0xAC) = value_rows;

    if (value_rows > 0) {
        std::int32_t i = 0;
        do {
            const std::int32_t kind = row_kinds[i];
            void* row_template = at<void*>(screen, 0x90);
            if (kind == 1) {
                row_template = at<void*>(screen, 0x9C);
            } else if (kind == 2) {
                row_template = at<void*>(screen, 0x98);
            } else if (kind == 3) {
                row_template = at<void*>(screen, 0x94);
            }
            void* row = host.widget_clone_subtree_00aab4c0(row_template, nullptr);
            host.gui_widget_set_listener_00aa6bc0(row, self + 0xC, 0);
            at<std::int32_t>(row, 0xD8) = 1;
            if (row_kinds[i] == 3 || row_kinds[i] == 1) {
                with_child(row, 0x00CEB278u, 0x0F, host,
                           [&](void* child) { host.gui_widget_set_listener_00aa6bc0(child, self + 0xC, 0); });
                with_child(row, 0x00CEB264u, 0x10, host,
                           [&](void* child) { host.gui_widget_set_listener_00aa6bc0(child, self + 0xC, 0); });
            }
            {
                Cc13SweepBNativeString name;
                build_literal(name, 0x00CEE4CCu, 9, host);
                Cc13SweepBNativeString label;
                void* text = host.options_row_label_at_005f0220(&label, labels, i);
                host.gui_text_set_localised_source_00abaed0(host.widget_find_child_00aa7e00(row, &name, 1), text, 1);
                release_string(label, host);
                release_string(name, host);
            }
            if (i != 0 && static_cast<const void*>(row_kinds) == host.image_global(0x00CF385Cu) &&
                at<std::uint8_t>(screen, 0x188) == 0) {
                at<std::uint8_t>(row, 0x77) = 1;
                host.gui_widget_set_listener_00aa6bc0(row, nullptr, 0);
            }
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x88), row, nullptr, 0);
            host.widget_vcall_34_set_visible(row, 1);
            ++i;
        } while (i < value_rows);
    }

    if (list_pages) {
        for (std::int32_t i = value_rows; i < label_count; ++i) {
            void* row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x8C), nullptr);
            Cc13SweepBNativeString label;
            void* text = host.options_row_label_at_005f0220(&label, labels, i);
            host.gui_text_set_localised_source_00abaed0(row, text, 1);
            release_string(label, host);
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x84), row, nullptr, 0);
            host.widget_vcall_34_set_visible(row, 1);
        }
    }

    if (image_pointer(host, 0x00E198C4) != nullptr) {
        const std::int32_t mode = *static_cast<std::int32_t*>(host.image_global(0x00E19650));
        if (mode == 1) {
            host.options_screen_show_value_arrows_005f1620(screen, host.listbox_row_00a9be00(at<void*>(screen, 0x88), 0));
        } else if (mode == 3) {
            host.options_screen_show_value_arrows_005f1620(screen, host.listbox_row_00a9be00(at<void*>(screen, 0x88), 0));
            host.options_screen_show_value_arrows_005f1620(screen, host.listbox_row_00a9be00(at<void*>(screen, 0x88), 1));
            host.options_screen_show_value_arrows_005f1620(screen, host.listbox_row_00a9be00(at<void*>(screen, 0x88), 5));
        }
    }

    if (list_pages) {
        host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x84), self + 8);
        const std::int32_t page = at<std::int32_t>(screen, 0xA8);
        host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x84),
                                           static_cast<std::int32_t*>(host.image_global(0x00E19630u))[page]);
        host.widget_vcall_80(at<void*>(screen, 0x84), 1);
    }
    void* listener = self + 8;
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x88), listener);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x88), 1);
    at<std::uint8_t>(at<void*>(screen, 0x88), 0x14A) = 0;

    if (list_pages) {
        host.gui_listbox_clear_other_two_00a9cdf0(at<void*>(screen, 0x84), at<void*>(screen, 0x84),
                                                  at<void*>(screen, 0x88));
        Cc13SweepBNativeString s30, s28, s10, s18, s20;
        build_literal(s30, 0x00CE3A0Cu, 0, host);
        build_literal(s28, 0x00CEB3A8u, 0x0C, host);
        build_literal(s10, 0x00CE3A0Cu, 0, host);
        build_literal(s18, 0x00CE3A0Cu, 0, host);
        build_literal(s20, 0x00CEB3ECu, 0x0E, host);
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {static_cast<std::uint32_t>(-0x5E), &s20, 0},
            {0, &s18, 1},
            {0, &s10, 1},
            {static_cast<std::uint32_t>(-0x5D), &s28, 2},
            {0, &s30, 1},
        };
        host.command_bar_rebuild_0054b530(image_pointer(host, 0x00E1930C), triples);
        release_string(s20, host);
        release_string(s18, host);
        release_string(s10, host);
        release_string(s28, host);
        release_string(s30, host);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x254), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x258), 0);
    } else {
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
    }
    if (at<std::uint8_t>(screen, 0x251) != 0) {
        at<std::uint8_t>(screen, 0x251) = 0;
    } else {
        at<std::uint8_t>(screen, 0x250) = 0;
    }
}

namespace {

// The 4Ch-byte camera path point record built inline at 007A5AB0 and
// 007A5B77: 00BF681B(4Ch); when non-null +4h/+8h/+0Ch = 0, byte +10h = 0,
// vtable 00D043D8, +14h = 0, floats +18h..+20h = 0, +24h = 0, floats
// +28h..+38h = 0, +48h = 0, +40h = +3Ch = 1/3 (00D04678, 0x3EAAAAAB), +44h = 0.
void* new_camera_path_point(Cc13SweepB6Host& host) {
    void* point = host.crt_operator_new_00bf681b(0x4C);
    if (point == nullptr) {
        return nullptr;
    }
    const float third = cc13_sweepB_f32(0x3EAAAAABu);
    at<std::uint32_t>(point, 0x04) = 0;
    at<std::uint32_t>(point, 0x08) = 0;
    at<std::uint32_t>(point, 0x0C) = 0;
    at<std::uint8_t>(point, 0x10) = 0;
    at<std::uint32_t>(point, 0x00) = 0x00D043D8u;  // the image's vtable address, kept as a value
    at<std::uint32_t>(point, 0x14) = 0;
    at<float>(point, 0x18) = 0.0f;
    at<float>(point, 0x1C) = 0.0f;
    at<float>(point, 0x20) = 0.0f;
    at<std::uint32_t>(point, 0x24) = 0;
    at<float>(point, 0x28) = 0.0f;
    at<float>(point, 0x2C) = 0.0f;
    at<float>(point, 0x30) = 0.0f;
    at<float>(point, 0x34) = 0.0f;
    at<float>(point, 0x38) = 0.0f;
    at<std::uint32_t>(point, 0x48) = 0;
    at<float>(point, 0x40) = third;
    at<float>(point, 0x3C) = third;
    at<float>(point, 0x44) = 0.0f;
    return point;
}

std::uint32_t point_count(void* camera) {
    return vector_count(camera, 0x47C);
}

}  // namespace

// ---------------------------------------------------------------------------
// 007A5110 BSP_MovCam_EnterMode
// ABI: __thiscall(ECX = camera, [esp+4] char snap), RET 4. EH frame (handler
// 00C8BA66; state 0/1 around the temporary CameraRef at [esp+34h];
// unwinding not modelled). 18 callers (the mode setters and 007A5A00).
// Frame: [esp+10h]/[esp+14h]/[esp+18h] float scratch, +1Ch/+28h/+74h/+80h/
// +8Ch float[3]s, +34h a 4x4 (or the temporary CameraRef), +98h/+0D4h/+118h/
// +154h 4x4 matrices.
//
// Prelude: byte +4B4h = 0; +3ACh/+3B0h/+3B4h/+3D4h/+3D8h = 0.0f; +460h =
// 1.0f; when +1BCh is set 00B6FBB0(+1BCh, [00F889B4]) and 00B6FBF0(+1BCh,
// 2.0f). +3E4h = 0 unless it equals +390h or +38Ch and the mode +384h is
// neither 0 nor 5.
// "View copy" below = 00B6DB70(view) when byte view+5Ch lacks bit 2, then
// 004134F0(+74h, view+0F0h) with view = [[00E188A8]+19FCh], then 00794360.
// Mode 0: view copy. Mode 1: no target +38Ch -> mode 0 and return without
// the tail; else s = float([[t+538h]+0A0h] * 21.0), view copy, d = t pos
// (+0FCh..; 00414DB0 when byte +0C8h is clear) - camera pos +394h, len =
// float(0042B2F0(d)); without snap the tail; with snap d is taken again, f =
// float(1 - s * 0.25 / len) (fld1; fsubrp), camera pos += d * f (each product
// and sum a float store; y as pos + dy*f), the target's vtable +34h(&tmp)
// gives +3A0h..+3A8h, 00794130(camera, t, -1.0f), +3E4h = t, +3E8h = 0,
// then 007904D0, 00799750, 00794000.
// Mode 2: no target -> mode 0 and return; +3E4h = 0 when it is the target;
// 007A4EF0(camera, t) (the cc13_sweepB2 reconstruction is called);
// *007A0AC0([+4D0h]+64h, &+38Ch) = [+388h]; with the class camera array
// a = [[t+538h]+0BCh] non-null and [a+4] > [+388h] (signed): 004134F0(+420h,
// 0047E540(&m, 005CD820(a, i)+38h)), then 005CC5E0([+4D0h], a by-value copy
// (00426060) of the native string at 005CD820(a, i)); else +420h = identity
// with m[14] = [[t+538h]+0A0h]. Angles +3C4h/+3C8h = 00418170(007A4E10(
// [+4D0h]+70h, &+38Ch), [+388h]); t pose refreshed; +74h = RotX(+3C4h) *
// RotY(+3C8h) * (+420h) * t+0CCh (0042B450, 0042B490, 00413920 x3, 004134F0);
// byte +490h = 1; 00B6FBF0(+1BCh, 0.1f) when +1BCh is set.
// Modes 3, 4, 6: view copy. Mode 3: +3E4h = 0, +418h/+41Ch = +3C4h/+3C8h,
// then +418h = -pi/2 (0xBFC90FDB); +3F0h = 00793920(&tmp, t, t ? (0,0,0) :
// camera+0A4h) via 00790320, tmp destroyed (00695870); +40Ch = float([[
// 00E198C4]+98h]+5Ch] + +40Ch). Mode 6: v = 0042AE20(00414260(00414260(
// 0042D7E0(t)+20h, [+488h]), [[+4D0h]+54h]), [+460h]); w = 00427EB0(t) + v
// (004142A0); n = 00419510(w - 00427EB0([+390h])); point = (n * k + w) with
// k = [+488h] (float stores; y as w.y + n.y*k); +3F0h = 00793920(&tmp, 0,
// point) via 00790320, tmp destroyed; +40Ch = float([+48Ch] + +40Ch).
// Then (all three) with snap: +394h.. = 00793AC0(+3F0h, &out, 1); the target's
// vtable +34h(&out) gives +3A0h..; mode 3: +3C4h/+3C8h = +418h/+41Ch; else
// e = +390h or +38Ch, 00794130(camera, e, -1.0f), +3E4h = e, +418h/+41Ch =
// +3C4h/+3C8h; then 00794000 and 00799750.
// Mode 5: +474h set -> 007972F0; else 007B04C0([+464h], [+468h], +394h,
// &v, 0) and 00794070(camera, 004142A0(+394h, v) by value); then 00794000 and
// 00799750. Other modes: nothing.
// Tail (every path except the two early returns): +170h vtable +0Ch;
// 00445790([[00E188A8]+21C4h], camera). No RNG draws.
void cc13_sweepB6_movcam_enter_mode_007a5110(void* camera, std::uint8_t snap, Cc13SweepB6Host& host) {
    auto* const cam = static_cast<std::uint8_t*>(camera);
    auto f = [&](std::size_t offset) -> float& { return at<float>(camera, offset); };
    at<std::uint8_t>(camera, 0x4B4) = 0;
    f(0x3AC) = 0.0f;
    f(0x3B0) = 0.0f;
    f(0x3B4) = 0.0f;
    f(0x3D4) = 0.0f;
    f(0x3D8) = 0.0f;
    f(0x460) = 1.0f;
    if (at<void*>(camera, 0x1BC) != nullptr) {
        host.camera_set_field_of_view_00b6fbb0(at<void*>(camera, 0x1BC),
                                               *static_cast<const float*>(host.image_global(0x00F889B4u)));
        host.camera_set_near_plane_00b6fbf0(at<void*>(camera, 0x1BC), 2.0f);
    }
    {
        void* held = at<void*>(camera, 0x3E4);
        bool keep = false;
        if (at<void*>(camera, 0x390) == held || at<void*>(camera, 0x38C) == held) {
            const std::int32_t mode = at<std::int32_t>(camera, 0x384);
            keep = mode != 0 && mode != 5;
        }
        if (!keep) {
            at<void*>(camera, 0x3E4) = nullptr;
        }
    }
    auto view_copy = [&] {
        void* view = at<void*>(image_pointer(host, 0x00E188A8), 0x19FC);
        if ((at<std::uint8_t>(view, 0x5C) & 2) == 0) {
            host.transform_refresh_world_00b6db70(view);
        }
        host.matrix_copy_4x4_004134f0(&f(0x74), &at<float>(view, 0xF0));
        host.movcam_seed_from_mover_00794360(camera);
    };
    auto refreshed = [&](void* entity) {
        if (at<std::uint8_t>(entity, 0xC8) == 0) {
            host.entity_pose_refresh_world_00414db0(entity);
        }
        return entity;
    };
    auto set_velocity = [&](float* scratch) {
        const float* v = host.unit_vcall_34_velocity(at<void*>(camera, 0x38C), scratch);
        f(0x3A0) = v[0];
        f(0x3A4) = v[1];
        f(0x3A8) = v[2];
    };

    const std::int32_t mode = at<std::int32_t>(camera, 0x384);
    if (mode == 0) {
        view_copy();
    } else if (mode == 1) {
        void* target = at<void*>(camera, 0x38C);
        if (target == nullptr) {
            at<std::int32_t>(camera, 0x384) = 0;
            return;
        }
        const float s = static_cast<float>(static_cast<double>(at<float>(at<void*>(target, 0x538), 0xA0)) * 21.0);
        view_copy();
        void* t = refreshed(at<void*>(camera, 0x38C));
        float d[3] = {static_cast<float>(static_cast<double>(at<float>(t, 0xFC)) - f(0x394)),
                      static_cast<float>(static_cast<double>(at<float>(t, 0x100)) - f(0x398)),
                      static_cast<float>(static_cast<double>(at<float>(t, 0x104)) - f(0x39C))};
        const float len = static_cast<float>(host.vector3_length_0042b2f0(d));
        if (snap != 0) {
            t = refreshed(at<void*>(camera, 0x38C));
            const float ex = static_cast<float>(static_cast<double>(at<float>(t, 0xFC)) - f(0x394));
            const float ey = static_cast<float>(static_cast<double>(at<float>(t, 0x100)) - f(0x398));
            const float ez = static_cast<float>(static_cast<double>(at<float>(t, 0x104)) - f(0x39C));
            const float k = static_cast<float>(1.0 - static_cast<double>(s) * 0.25 / len);
            float m[3] = {static_cast<float>(static_cast<double>(ex) * k), static_cast<float>(static_cast<double>(ey) * k),
                          static_cast<float>(static_cast<double>(k) * ez)};
            f(0x394) = static_cast<float>(static_cast<double>(m[0]) + f(0x394));
            f(0x398) = static_cast<float>(static_cast<double>(f(0x398)) + m[1]);
            f(0x39C) = static_cast<float>(static_cast<double>(m[2]) + f(0x39C));
            set_velocity(m);
            host.camera_position_aim_at_target_offset_00794130(camera, at<void*>(camera, 0x38C), -1.0f);
            at<void*>(camera, 0x3E4) = at<void*>(camera, 0x38C);
            f(0x3E8) = 0.0f;
            host.movcam_wrap_and_clamp_angles_007904d0(camera);
            host.movcam_apply_position_and_angles_00799750(camera);
            host.movcam_reset_motion_00794000(camera);
        }
    } else if (mode == 2) {
        void* const* key = &at<void*>(camera, 0x38C);
        void* target = *key;
        if (target == nullptr) {
            at<std::int32_t>(camera, 0x384) = 0;
            return;
        }
        if (at<void*>(camera, 0x3E4) == target) {
            at<void*>(camera, 0x3E4) = nullptr;
        }
        cc13_sweepB2_movcam_seed_angles_from_vehicle_camera_007a4ef0(camera, target, host);
        *host.map_subscript_007a0ac0(at<std::uint8_t*>(camera, 0x4D0) + 0x64, key) = at<std::int32_t>(camera, 0x388);
        void* cls = at<void*>(*key, 0x538);
        void* cameras = at<void*>(cls, 0xBC);
        float m34[16];
        if (cameras != nullptr && at<std::int32_t>(cameras, 4) > at<std::int32_t>(camera, 0x388)) {
            auto* element = static_cast<std::uint8_t*>(host.vehicle_camera_array_at_005cd820(cameras, at<std::int32_t>(camera, 0x388)));
            host.matrix_copy_4x4_004134f0(&f(0x420), host.transform_init_translation_0047e540(m34, reinterpret_cast<const float*>(element + 0x38)));
            void* label = host.vehicle_camera_array_at_005cd820(at<void*>(at<void*>(*key, 0x538), 0xBC),
                                                                at<std::int32_t>(camera, 0x388));
            Cc13SweepBNativeString text;
            host.native_string_copy_construct_00426060(&text, label);
            host.movcam_show_mode_text_005cc5e0(at<void*>(camera, 0x4D0), text);
        } else {
            const float h = at<float>(cls, 0xA0);
            const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, h,    1.0f};
            std::memcpy(m34, identity, sizeof m34);
            host.matrix_copy_4x4_004134f0(&f(0x420), m34);
        }
        const float* angles = host.vector8_at_00418170(
            host.map_subscript_007a4e10(at<std::uint8_t*>(camera, 0x4D0) + 0x70, key),
            static_cast<std::uint32_t>(at<std::int32_t>(camera, 0x388)));
        f(0x3C4) = angles[0];
        f(0x3C8) = angles[1];
        void* t = refreshed(*key);
        float m98[16];
        float m154[16];
        float md4[16];
        float m118[16];
        const float* ry = host.matrix_set_rotation_y_negated_0042b490(m98, f(0x3C8));
        const float* rx = host.matrix_set_rotation_x_negated_0042b450(m154, f(0x3C4));
        const float* a = host.matrix_multiply_00413920(rx, md4, ry);
        const float* b = host.matrix_multiply_00413920(a, m118, &f(0x420));
        const float* c = host.matrix_multiply_00413920(b, m34, &at<float>(t, 0xCC));
        host.matrix_copy_4x4_004134f0(&f(0x74), c);
        at<std::uint8_t>(camera, 0x490) = 1;
        if (at<void*>(camera, 0x1BC) != nullptr) {
            host.camera_set_near_plane_00b6fbf0(at<void*>(camera, 0x1BC), cc13_sweepB_f32(0x3DCCCCCDu));
        }
    } else if (mode == 3 || mode == 4 || mode == 6) {
        view_copy();
        alignas(8) std::uint8_t ref[0x40];
        const std::int32_t current = at<std::int32_t>(camera, 0x384);
        if (current == 3) {
            at<void*>(camera, 0x3E4) = nullptr;
            f(0x418) = f(0x3C4);
            f(0x41C) = f(0x3C8);
            void* target = at<void*>(camera, 0x38C);
            f(0x418) = cc13_sweepB_f32(0xBFC90FDBu);
            const float zero[3] = {0.0f, 0.0f, 0.0f};
            const float* point = target != nullptr ? zero : &f(0xA4);
            host.camera_ref_assign_00790320(cam + 0x3F0, host.camera_ref_construct_point_00793920(ref, target, point));
            host.callback_owner_destroy_00695870(ref);
            const float v = at<float>(at<void*>(image_pointer(host, 0x00E198C4), 0x98), 0x5C);
            f(0x40C) = static_cast<float>(static_cast<double>(v) + f(0x40C));
        } else if (current == 6) {
            const float* world = host.entity_get_world_matrix_0042d7e0(at<void*>(camera, 0x38C));
            float v74[3];
            float v8c[3];
            float v1c[3];
            float v28[3];
            const float* a = host.vector3_scale_to_00414260(world + 8, v74, f(0x488));
            const float* b = host.vector3_scale_to_00414260(a, v8c, at<float>(at<void*>(camera, 0x4D0), 0x54));
            const float* c = host.vector3_divide_scalar_to_0042ae20(b, v1c, f(0x460));
            host.vector3_add_to_004142a0(host.entity_world_position_00427eb0(at<void*>(camera, 0x38C)), v28, c);
            const float* tp = host.entity_world_position_00427eb0(at<void*>(camera, 0x390));
            v1c[0] = static_cast<float>(static_cast<double>(v28[0]) - tp[0]);
            v1c[1] = static_cast<float>(static_cast<double>(v28[1]) - tp[1]);
            v1c[2] = static_cast<float>(static_cast<double>(v28[2]) - tp[2]);
            const float* n = host.vector3_normalize_00419510(v74, v1c);
            const float k = f(0x488);
            const float x = static_cast<float>(static_cast<double>(n[0]) * k);
            const float y = static_cast<float>(static_cast<double>(n[1]) * k);
            const float z = static_cast<float>(static_cast<double>(k) * n[2]);
            v1c[0] = static_cast<float>(static_cast<double>(x) + v28[0]);
            v1c[1] = static_cast<float>(static_cast<double>(v28[1]) + y);
            v1c[2] = static_cast<float>(static_cast<double>(v28[2]) + z);
            host.camera_ref_assign_00790320(cam + 0x3F0, host.camera_ref_construct_point_00793920(ref, nullptr, v1c));
            host.callback_owner_destroy_00695870(ref);
            f(0x40C) = static_cast<float>(static_cast<double>(f(0x48C)) + f(0x40C));
        }
        if (snap != 0) {
            float out[3];
            const float* p = host.camera_ref_resolve_00793ac0(cam + 0x3F0, out, 1);
            f(0x394) = p[0];
            f(0x398) = p[1];
            f(0x39C) = p[2];
            set_velocity(out);
            if (at<std::int32_t>(camera, 0x384) == 3) {
                f(0x3C4) = f(0x418);
                f(0x3C8) = f(0x41C);
            } else {
                void* aim = at<void*>(camera, 0x390);
                if (aim == nullptr) {
                    aim = at<void*>(camera, 0x38C);
                }
                host.camera_position_aim_at_target_offset_00794130(camera, aim, -1.0f);
                at<void*>(camera, 0x3E4) = aim;
                f(0x418) = f(0x3C4);
                f(0x41C) = f(0x3C8);
            }
            host.movcam_reset_motion_00794000(camera);
            host.movcam_apply_position_and_angles_00799750(camera);
        }
    } else if (mode == 5) {
        if (at<void*>(camera, 0x474) != nullptr) {
            host.camera_position_update_path_007972f0(camera);
        } else {
            float v80[3];
            host.camera_path_sample_world_007b04c0(at<void*>(camera, 0x464), f(0x468), &f(0x394), v80, 0);
            float point[3];
            host.vector3_add_to_004142a0(&f(0x394), point, v80);
            host.camera_position_aim_at_point_00794070(camera, point[0], point[1], point[2]);
        }
        host.movcam_reset_motion_00794000(camera);
        host.movcam_apply_position_and_angles_00799750(camera);
    }
    host.movcam_controller_vcall_0c(cam + 0x170);
    host.weather_config_apply_camera_shake_00445790(at<void*>(image_pointer(host, 0x00E188A8), 0x21C4), camera);
}

// ---------------------------------------------------------------------------
// 007A6200 (no Ghidra function: the bytes 007A6200..007A6267 sit after
// 007A5A00's RET 14h inside its Ghidra body; one rel32 caller, 007A8203 in
// BSP_MovCam_PickDeckPosition 007A8180).
// ABI: __thiscall(ECX = camera, [esp+4] int index), RET 4.
// index == -1: unless the mode +384h is already 1, mode = 1, +388h = 0 and
// 007A5110(camera, 0). Otherwise, unless mode 2 with +388h == index is already
// set, mode = 2, +388h = index and 007A5110(camera, 0). Always: +170h vtable
// +0Ch. (007A5110 is the reconstruction above.) No RNG draws.
void cc13_sweepB6_movcam_select_deck_camera_007a6200(void* camera, std::int32_t index, Cc13SweepB6Host& host) {
    if (index == -1) {
        if (at<std::int32_t>(camera, 0x384) != 1) {
            at<std::int32_t>(camera, 0x384) = 1;
            at<std::int32_t>(camera, 0x388) = 0;
            cc13_sweepB6_movcam_enter_mode_007a5110(camera, 0, host);
        }
    } else if (at<std::int32_t>(camera, 0x384) != 2 || at<std::int32_t>(camera, 0x388) != index) {
        at<std::int32_t>(camera, 0x384) = 2;
        at<std::int32_t>(camera, 0x388) = index;
        cc13_sweepB6_movcam_enter_mode_007a5110(camera, 0, host);
    }
    host.movcam_controller_vcall_0c(static_cast<std::uint8_t*>(camera) + 0x170);
}

// ---------------------------------------------------------------------------
// 007A5A00 BSP_MovCam_BeginPathWalk
// ABI: __thiscall(ECX = camera, [esp+4] float start, [esp+8] char keep_points,
// [esp+0Ch] float duration, [esp+10h] const native string* path_name,
// [esp+14h] variants), RET 14h. EH frame (handler 00C8BA9C; state 0 around
// the path block construction, 1..3 around the interpolator and its name;
// unwinding not modelled). Frame after the pushes: [esp+10h] the name-built
// flag, +14h the "moveSpeed" native string, +1Ch direction / point, +28h
// out vector, +34h a 4x4 matrix; the argument slots +90h/+94h/+8Ch are
// reused as float scratch, which the C++ keeps in locals. Callers 007A7200,
// 007A74F0, 007A7C60. The bytes 007A6200..007A6267 after the RET belong to
// another routine.
//
// Path name (when its length is non-zero, unsigned): unless it is the
// camera's own +49Ch, 0041DD40(+49Ch, length, 1), then when the length is
// non-zero 00BF7680([+4A0h], name data, [+49Ch]) (no terminator byte). The
// old variant vector +4ACh, when set: 006B88E0(v, 0), 00BF6989([v]),
// 00BF65AC(v), +4ACh = 0; then +4ACh = variants. Always: +384h = 5.
// Without keep_points: 00794360(camera); point 0 is a new record at the
// camera position (+394h..+39Ch), +48h = 4, +34h/+38h = +3C4h/+3C8h, stored
// into [+47Ch][0] (size != 0 checked). Point 1: direction = +3A0h when
// 0042B2F0(+3A0h) > 1.0 (fld1; fcomip; jbe, so NaN falls through), else
// 00793730(&tmp, +3A0h, 1.0f) when the length > (double)1e-4f (00D7A268), else
// +94h..+9Ch. d = camera position - 00793F10([+47Ch][2], &out, 1) (size > 2
// checked; float stores); s = float(k * (float(|d|) * 0.125)) with k =
// [[+4D0h]+44h] and 0.125 the double 00D04380; point = position + s * dir
// (each product and sum stored as float; the y product is dir.y * s), +48h = 0,
// stored into [+47Ch][1] (size > 1 checked).
// Path: block = 00BF55BE(210h), 00BF79F0(block, 0, 210h), path = 0047B660(
// block, 0) or null; +474h = path + 1E4h or null; entity = 007ACA30(+474h);
// entity vtable +98h(0, [camera+30h], identity 4x4).
// Points: speed = [[+4D0h]+34h]; for each record r in +47Ch: p = r+18h when
// r+14h is null, else (00414DB0 when byte [r+14h]+0C8h is clear) the owner's
// matrix +0CCh with row 1 replaced by 00F8758C..00F87594, 0085DAD0, and
// 004142E0(r+18h, &out, matrix); 007AD330(+474h, p); then r+44h: comiss
// 0, v; jb -> speed = v (taken when v > 0 or NaN); else r+44h = speed.
// Then 007AF150(+474h) when byte [+474h]+18h is clear; +464h = +474h, +468h =
// 0; t = float(007B05E0(+474h, count - 1)); +470h = start; +46Ch = t.
// duration > 0 (comiss; jbe skips NaN): rate = float(|float(t - [+468h])| /
// duration). keep_points: 005CB9F0([+4D0h], rate). Otherwise an interpolator:
// 00BF681B(18h); when non-null the name "moveSpeed" (00D04610) is built by
// 0041E870 and 007900B0(obj, &name, float(0042B2F0(+3A0h)), rate, 0.0f, k);
// the result (or null) is appended to the list +4D4h (00796930(list, head,
// [head+4], &value), 0079DE40(list, 1), head+4 = node, [node+4]+0 = node);
// the name is released when it was built.
// Last: 007A5110(camera, keep_points). The image pushes the whole argument
// dword; the C++ passes the byte zero-extended. No RNG draws.
void cc13_sweepB6_movcam_begin_path_walk_007a5a00(void* camera, float start, std::uint8_t keep_points,
                                                  float duration, const void* path_name, void* variants,
                                                  Cc13SweepB6Host& host) {
    auto* const cam = static_cast<std::uint8_t*>(camera);
    if (*static_cast<const std::uint32_t*>(path_name) != 0) {
        void* own = cam + 0x49C;
        if (own != path_name) {
            host.native_string_resize_0041dd40(own, *static_cast<const std::uint32_t*>(path_name), 1);
            if (*static_cast<const std::uint32_t*>(path_name) != 0) {
                host.crt_memcpy_00bf7680(at<void*>(camera, 0x4A0),
                                         static_cast<const Cc13SweepBNativeString*>(path_name)->data,
                                         at<std::uint32_t>(camera, 0x49C));
            }
        }
        void* old = at<void*>(camera, 0x4AC);
        if (old != nullptr) {
            host.lua_variant_vector_resize_006b88e0(old, 0);
            host.crt_free_00bf6989(at<void*>(old, 0));
            host.crt_operator_delete_00bf65ac(old);
            at<void*>(camera, 0x4AC) = nullptr;
        }
        at<void*>(camera, 0x4AC) = variants;
    }
    at<std::int32_t>(camera, 0x384) = 5;

    if (keep_points == 0) {
        host.movcam_seed_from_mover_00794360(camera);
        void* first_point = new_camera_path_point(host);
        at<float>(first_point, 0x18) = at<float>(camera, 0x394);
        at<float>(first_point, 0x1C) = at<float>(camera, 0x398);
        at<float>(first_point, 0x20) = at<float>(camera, 0x39C);
        at<std::int32_t>(first_point, 0x48) = 4;
        at<float>(first_point, 0x34) = at<float>(camera, 0x3C4);
        at<float>(first_point, 0x38) = at<float>(camera, 0x3C8);
        if (point_count(camera) == 0) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        at<void**>(camera, 0x47C)[0] = first_point;

        void* second_point = new_camera_path_point(host);
        const float* lane = &at<float>(camera, 0x3A0);
        float dir[3];
        if (host.vector3_length_0042b2f0(lane) > 1.0) {
            dir[0] = lane[0];
            dir[1] = lane[1];
            dir[2] = lane[2];
        } else if (host.vector3_length_0042b2f0(lane) > static_cast<double>(cc13_sweepB_f32(0x38D1B717u))) {
            float tmp[3];
            const float* scaled = host.vector3_scale_to_length_00793730(tmp, lane, 1.0f);
            dir[0] = scaled[0];
            dir[1] = scaled[1];
            dir[2] = scaled[2];
        } else {
            dir[0] = at<float>(camera, 0x94);
            dir[1] = at<float>(camera, 0x98);
            dir[2] = at<float>(camera, 0x9C);
        }
        if (point_count(camera) <= 2) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        float out[3];
        const float* ref = host.camera_ref_transform_to_world_00793f10(at<void**>(camera, 0x47C)[2], out, 1);
        const float dx = static_cast<float>(static_cast<double>(at<float>(camera, 0x394)) - ref[0]);
        const float dy = static_cast<float>(static_cast<double>(at<float>(camera, 0x398)) - ref[1]);
        const float dz = static_cast<float>(static_cast<double>(at<float>(camera, 0x39C)) - ref[2]);
        out[0] = dx;
        out[1] = dy;
        out[2] = dz;
        const float length = static_cast<float>(host.vector3_length_0042b2f0(out));
        const float k = at<float>(at<void*>(camera, 0x4D0), 0x44);
        const float s = static_cast<float>(static_cast<double>(k) * (static_cast<double>(length) * 0.125));
        const float sx = static_cast<float>(static_cast<double>(s) * dir[0]);
        const float sy = static_cast<float>(static_cast<double>(dir[1]) * s);
        const float sz = static_cast<float>(static_cast<double>(s) * dir[2]);
        const float px = static_cast<float>(static_cast<double>(sx) + at<float>(camera, 0x394));
        const float py = static_cast<float>(static_cast<double>(at<float>(camera, 0x398)) + sy);
        const float pz = static_cast<float>(static_cast<double>(at<float>(camera, 0x39C)) + sz);
        at<float>(second_point, 0x18) = px;
        at<float>(second_point, 0x1C) = py;
        at<float>(second_point, 0x20) = pz;
        at<std::int32_t>(second_point, 0x48) = 0;
        if (point_count(camera) <= 1) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        at<void**>(camera, 0x47C)[1] = second_point;
    }

    void* block = host.crt_operator_new_00bf55be(0x210);
    host.crt_memset_00bf79f0(block, 0, 0x210);
    void* path = block != nullptr ? host.path_construct_0047b660(block, 0) : nullptr;
    at<void*>(camera, 0x474) = path != nullptr ? static_cast<std::uint8_t*>(path) + 0x1E4 : nullptr;
    void* entity = host.path_object_get_entity_007aca30(at<void*>(camera, 0x474));
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    host.entity_vcall_98_restore_parent(entity, 0, at<std::uint32_t>(camera, 0x30), identity);

    float speed = at<float>(at<void*>(camera, 0x4D0), 0x34);
    for (std::uint32_t i = 0;; ++i) {
        if (point_count(camera) <= i) {
            break;
        }
        if (at<std::uint32_t>(camera, 0x47C) == 0 || i >= point_count(camera)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* point = at<void**>(camera, 0x47C)[i];
        void* owner = at<void*>(point, 0x14);
        float p[3];
        if (owner == nullptr) {
            p[0] = at<float>(point, 0x18);
            p[1] = at<float>(point, 0x1C);
            p[2] = at<float>(point, 0x20);
        } else {
            if (at<std::uint8_t>(owner, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(owner);
            }
            float matrix[16];
            std::memcpy(matrix, static_cast<std::uint8_t*>(owner) + 0xCC, sizeof matrix);
            matrix[4] = *static_cast<const float*>(host.image_global(0x00F8758Cu));
            matrix[5] = *static_cast<const float*>(host.image_global(0x00F87590u));
            matrix[6] = *static_cast<const float*>(host.image_global(0x00F87594u));
            host.matrix_orthonormalize_from_row_y_then_z_0085dad0(matrix);
            float world[3];
            host.vector3_transform_affine_point_004142e0(&at<float>(point, 0x18), world, matrix);
            p[0] = world[0];
            p[1] = world[1];
            p[2] = world[2];
        }
        host.path_points_add_point_local_007ad330(at<void*>(camera, 0x474), p);
        const float v = at<float>(point, 0x44);
        if (0.0f >= v) {
            at<float>(point, 0x44) = speed;
        } else {
            speed = v;
        }
    }

    if (at<std::uint8_t>(at<void*>(camera, 0x474), 0x18) == 0) {
        host.camera_path_derive_knots_007af150(at<void*>(camera, 0x474));
    }
    void* points = at<void*>(camera, 0x474);
    at<void*>(camera, 0x464) = points;
    at<float>(camera, 0x468) = 0.0f;
    const std::int32_t count = static_cast<std::int32_t>(point_count(camera));
    const float t = static_cast<float>(host.path_parameter_at_007b05e0(points, count - 1));
    at<float>(camera, 0x470) = start;
    at<float>(camera, 0x46C) = t;
    if (duration > 0.0f) {
        const float span = std::fabs(static_cast<float>(static_cast<double>(t) - at<float>(camera, 0x468)));
        const float rate = static_cast<float>(static_cast<double>(span) / duration);
        if (keep_points != 0) {
            host.movcam_set_linear_speed_limit_005cb9f0(at<void*>(camera, 0x4D0), rate);
        } else {
            void* interpolator = host.crt_operator_new_00bf681b(0x18);
            std::uint32_t built = 0;
            Cc13SweepBNativeString name{};
            void* made = nullptr;
            if (interpolator != nullptr) {
                host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(0x00D04610u)));
                const float k = at<float>(at<void*>(camera, 0x4D0), 0x44);
                built |= 1;
                const float lane_length = static_cast<float>(host.vector3_length_0042b2f0(&at<float>(camera, 0x3A0)));
                made = host.movcam_interpolator_init_007900b0(interpolator, &name, lane_length, rate, 0.0f, k);
            }
            void* list = cam + 0x4D4;
            void* head = at<void*>(camera, 0x4D8);
            void* node = host.list_buy_node_00796930(list, head, at<void*>(head, 4), &made);
            host.list_incsize_0079de40(list, 1);
            at<void*>(head, 4) = node;
            at<void*>(at<void*>(node, 4), 0) = node;
            if ((built & 1) != 0) {
                release_string(name, host);
            }
        }
    }
    cc13_sweepB6_movcam_enter_mode_007a5110(camera, keep_points, host);
}

// ---------------------------------------------------------------------------
// 005FA0F0 BSP_HudOrderScreen_BindLayout
// ABI: __thiscall(ECX = order screen), plain RET; EBP frame aligned to 8, EH
// frame (handler 00C76800, state 0 around each name; unwinding not
// modelled). No direct caller: vtable 00CF3B7C slot +14h (the dword at
// 00CF3B90, the only absolute reference).
// Twenty bindings in the image's order, each: the name is built from its
// literal (0041DD40 + 00BF7680), the field = 00AA7E00([screen+8], &name, 1),
// the name released. Nothing else runs. No RNG draws.
void cc13_sweepB6_hud_order_screen_bind_layout_005fa0f0(void* screen, Cc13SweepB6Host& host) {
    struct Binding {
        std::uint32_t literal;
        std::uint32_t length;
        std::size_t field;
    };
    static const Binding kBindings[] = {
        {0x00CED94Cu, 0x19, 0x0C},  // ship_plane_agressive_Icon
        {0x00CED930u, 0x19, 0x10},  // ship_plane_defensive_Icon
        {0x00CF3C7Cu, 0x14, 0x14},  // plane_formation_Icon
        {0x00CF3C68u, 0x13, 0x18},  // ship_formation_Icon
        {0x00CF3C50u, 0x14, 0x1C},  // ship_plane_move_Icon
        {0x00CF3C3Cu, 0x11, 0x20},  // plane_attack_Icon
        {0x00CF3C28u, 0x10, 0x24},  // ship_attack_Icon
        {0x00CF3C18u, 0x0E, 0x28},  // ship_sell_Icon
        {0x00CF3C00u, 0x14, 0x2C},  // ship_leave_form_Icon
        {0x00CF3BECu, 0x12, 0x30},  // plane_retreat_Icon
        {0x00CF3BD8u, 0x12, 0x34},  // plane_landing_Icon
        {0x00CED854u, 0x11, 0x3C},  // repair_glass_Icon
        {0x00CED840u, 0x11, 0x40},  // repair_frame_Icon
        {0x00CED8D8u, 0x0B, 0x38},  // cancel_Icon
        {0x00CED8C4u, 0x13, 0x44},  // repair_hl_west_Icon
        {0x00CED8B0u, 0x13, 0x48},  // repair_hl_east_Icon
        {0x00CED898u, 0x14, 0x4C},  // repair_hl_north_Icon
        {0x00CED880u, 0x14, 0x50},  // repair_hl_south_Icon
        {0x00CED868u, 0x15, 0x54},  // repair_hl_middle_Icon
        {0x00CED834u, 0x09, 0x58},  // info_Text
    };
    for (const Binding& binding : kBindings) {
        Cc13SweepBNativeString name;
        build_literal(name, binding.literal, binding.length, host);
        at<void*>(screen, binding.field) = host.widget_find_child_00aa7e00(at<void*>(screen, 8), &name, 1);
        release_string(name, host);
    }
}

// ---------------------------------------------------------------------------
// 0065D220 BSP_HudSupportManager_RebindSlots
// ABI: __thiscall(ECX = support-manager screen), plain RET; EBP frame aligned
// to 8, EH frame (handler 00C7B0FE, states 0..11h over the name strings;
// unwinding not modelled). Callers 006169F0, 0061B850, 0065DAA0, 00674B80.
// Frame: [esp+14h] the running offset v (float), [esp+60h] the screen,
// [esp+7Ch] the slot down-counter (4).
//
// +138h vtable +34h(1); v = [+2A8h]. For slot i = 0..3 (esi = screen+9Ch+4i):
//  +7Ch[i] = 00AAB4C0([+138h], 0);
//  +8Ch[i] = child "icon_L_Icon" (00CF4DDC, 0Bh) of +7Ch[i];
//  +0CCh[i] = child "circle_HL_Icon" (00CF4DC0, 0Eh) of +7Ch[i], then +34h(0);
//  +108h[i] = child "circle_small_01_Group" (00CF4DA8, 15h) of +7Ch[i];
//  +118h[i] = child "circle_small_02_Group" (00CF4D90, 15h) of +7Ch[i];
//  +9Ch[i] = child "number_Text" (00CF4DD0, 0Bh) of +8Ch[i];
//  +34h(0) on +9Ch[i], +108h[i], +118h[i];
//  vtable +44h(float) with A = [+2A4h]: +7Ch[i] gets A - v, +108h[i],
//  +118h[i] and +8Ch[i] get v - A (each difference stored as float; the
//  first is fld A; fsub v, the others fld v; fsub A);
//  +0ACh[i] = 00AAB4C0([+138h], 0); its "circle_small_01_Group" and
//  "circle_small_02_Group" children get +34h(0); +0BCh[i] = its
//  "icon_L_Icon" child; that one's "number_Text" child gets +34h(0);
//  +44h: +0ACh[i] gets A + v, +0BCh[i] gets -A - v (fchs; fsub v);
//  +0BCh[i] vtable +88h(3, 0, 1.0f);
//  +0DCh[i] = the "circle_HL_Icon" child of +0ACh[i], then +34h(0);
//  the "number_Text" child of +0BCh[i] gets +34h(0) again;
//  for j = 0..3: name = 004261A0(0043C130("circle_full_" (00CF4C4C),
//  004260B0(j)), "_Section" (00CF4C5C, 8)) -- temporaries released in the
//  image's order -- and the named children of +7Ch[i] and +0ACh[i] get +34h(0);
//  v = float(v + [+2A8h]).
// Last: +138h vtable +34h(0). Every name is built and released around its
// lookup as in the image. No RNG draws.
void cc13_sweepB6_hud_support_rebind_slots_0065d220(void* screen, Cc13SweepB6Host& host) {
    auto widget = [&](std::size_t offset) -> void*& { return at<void*>(screen, offset); };
    auto find = [&](void* parent, std::uint32_t literal, std::uint32_t length) {
        void* child = nullptr;
        with_child(parent, literal, length, host, [&](void* found) { child = found; });
        return child;
    };
    auto hide_child = [&](void* parent, std::uint32_t literal, std::uint32_t length) {
        with_child(parent, literal, length, host, [&](void* found) { host.widget_vcall_34_set_visible(found, 0); });
    };

    host.widget_vcall_34_set_visible(widget(0x138), 1);
    float v = at<float>(screen, 0x2A8);
    for (std::size_t i = 0; i < 4; ++i) {
        const std::size_t s = 4 * i;
        widget(0x7C + s) = host.widget_clone_subtree_00aab4c0(widget(0x138), nullptr);
        widget(0x8C + s) = find(widget(0x7C + s), 0x00CF4DDCu, 0x0B);
        widget(0xCC + s) = find(widget(0x7C + s), 0x00CF4DC0u, 0x0E);
        host.widget_vcall_34_set_visible(widget(0xCC + s), 0);
        widget(0x108 + s) = find(widget(0x7C + s), 0x00CF4DA8u, 0x15);
        widget(0x118 + s) = find(widget(0x7C + s), 0x00CF4D90u, 0x15);
        widget(0x9C + s) = find(widget(0x8C + s), 0x00CF4DD0u, 0x0B);
        host.widget_vcall_34_set_visible(widget(0x9C + s), 0);
        host.widget_vcall_34_set_visible(widget(0x108 + s), 0);
        host.widget_vcall_34_set_visible(widget(0x118 + s), 0);

        const float a = at<float>(screen, 0x2A4);
        host.widget_vcall_44_set_float(widget(0x7C + s), static_cast<float>(static_cast<double>(a) - v));
        host.widget_vcall_44_set_float(widget(0x108 + s),
                                       static_cast<float>(static_cast<double>(v) - at<float>(screen, 0x2A4)));
        host.widget_vcall_44_set_float(widget(0x118 + s),
                                       static_cast<float>(static_cast<double>(v) - at<float>(screen, 0x2A4)));
        host.widget_vcall_44_set_float(widget(0x8C + s),
                                       static_cast<float>(static_cast<double>(v) - at<float>(screen, 0x2A4)));

        widget(0xAC + s) = host.widget_clone_subtree_00aab4c0(widget(0x138), nullptr);
        hide_child(widget(0xAC + s), 0x00CF4DA8u, 0x15);
        hide_child(widget(0xAC + s), 0x00CF4D90u, 0x15);
        widget(0xBC + s) = find(widget(0xAC + s), 0x00CF4DDCu, 0x0B);
        hide_child(widget(0xBC + s), 0x00CF4DD0u, 0x0B);
        host.widget_vcall_44_set_float(widget(0xAC + s),
                                       static_cast<float>(static_cast<double>(at<float>(screen, 0x2A4)) + v));
        host.widget_vcall_44_set_float(widget(0xBC + s),
                                       static_cast<float>(-static_cast<double>(at<float>(screen, 0x2A4)) - v));
        host.widget_vcall_88(widget(0xBC + s), 3, 0, 1.0f);
        widget(0xDC + s) = find(widget(0xAC + s), 0x00CF4DC0u, 0x0E);
        host.widget_vcall_34_set_visible(widget(0xDC + s), 0);
        hide_child(widget(0xBC + s), 0x00CF4DD0u, 0x0B);

        for (std::int32_t j = 0; j < 4; ++j) {
            Cc13SweepBNativeString section;
            build_literal(section, 0x00CF4C5Cu, 8, host);
            Cc13SweepBNativeString number;
            void* number_text = host.native_string_from_int_004260b0(&number, j);
            Cc13SweepBNativeString prefixed;
            void* head = host.native_string_prefixed_0043c130(
                &prefixed, static_cast<const char*>(host.image_global(0x00CF4C4Cu)), number_text);
            Cc13SweepBNativeString name;
            host.native_string_concat_004261a0(head, &name, &section);
            release_string(prefixed, host);
            release_string(number, host);
            release_string(section, host);
            host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(widget(0x7C + s), &name, 1), 0);
            host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(widget(0xAC + s), &name, 1), 0);
            release_string(name, host);
        }
        v = static_cast<float>(static_cast<double>(v) + at<float>(screen, 0x2A8));
    }
    host.widget_vcall_34_set_visible(widget(0x138), 0);
}

}  // namespace bsp
