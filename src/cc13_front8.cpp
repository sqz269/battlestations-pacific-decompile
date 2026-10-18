// Packet cc13-front8 (marker cc13_front8): wiring-frontier callees, round 3.
// Evidence, ABI and uncertainty per function: docs/CC13_FRONT8.md. Nothing here is wired.
#include "bsp/cc13_front8.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace bsp::cc13_front8 {

namespace {

// A float from its image bit pattern (keeps the constants exact).
float f32(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// FILD m32 / FDIV by a float at 24-bit precision control: the exact quotient
// rounded once to a 24-bit significand (ties to even). The double quotient is
// rounded twice only when it lands exactly on a float midpoint; FMA's exact
// residual then decides the side.
float divide_int_by_float_pc24(std::int32_t n, float d) noexcept {
    const double q = static_cast<double>(n) / static_cast<double>(d);
    const float f = static_cast<float>(q);
    if (!std::isfinite(q) || static_cast<double>(f) == q) return f;
    const float g = std::nextafter(f, static_cast<double>(q) > static_cast<double>(f)
                                          ? std::numeric_limits<float>::infinity()
                                          : -std::numeric_limits<float>::infinity());
    const double mid = (static_cast<double>(f) + static_cast<double>(g)) * 0.5;
    if (q != mid) return f;
    const double residual = std::fma(mid, static_cast<double>(d), -static_cast<double>(n));  // mid*d - n
    if (residual == 0.0) {  // exact tie: even significand
        std::uint32_t fb;
        std::memcpy(&fb, &f, sizeof fb);
        return (fb & 1u) == 0 ? f : g;
    }
    // exact quotient < mid  <=>  mid*d > n for d > 0 (reversed for d < 0)
    const bool exact_below_mid = (residual > 0.0) == (d > 0.0f);
    const float low = f < g ? f : g;
    const float high = f < g ? g : f;
    return exact_below_mid ? low : high;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0077B280, __cdecl, 15 stack dwords (callers ADD ESP,3Ch), -> AL. Frame: SUB ESP,30h
// + EBP/ESI/EBX/EDI. Arguments: entity, record ({object, last tick}), byte flag, then
// pairs {float distance, int interval} from stack argument 4. Assembly 0077B280-
// 0077B6DD; x87 bytes read directly (DE FA at 0077B5EC is FDIVP ST(2),ST(0), i.e.
// elapsed / interval; DE C1 FADDP; D8 forms only otherwise).
//  1. Null record or null object -> 0. alive = byte [object+0C8h] (kept in the record
//     argument's slot). flag && !alive: entity+288h = 40h. Otherwise entity+288h = 0
//     and, when alive, !vslot5C(6), !flag and vslot15C() != byte [00F87150] -> 0.
//  2. time = float(vslot14([object+50h])) on [game+2078h], or [game+207Ch] when null.
//  3. (char)entity+288h >= 40h, or alive && vslot15C() == byte [00F87150]: level 40h
//     with band 6 when alive (else entity+288h unchanged, band 0) -> step 6.
//  4. I = float(n0); entity+288h = 0. Unless flag: V = position(00427EB0) - object+0ACh;
//     D = length(V); when D > 150.0 (00CE3808) and 007F7CD0(object): V /= D;
//     dot = (BC*Vy + B8*Vx) + C0*Vz; q = 007789D0(dot) / max(0.4, C4) (00415550);
//     D *= 00419010(0.5, C4 / float(pi/3) * 0.5, 1.2, 1.2, q) when 1.2 > q, else
//     00419010(1.2, 1.2, 3.0, 6.0, q). Band walk: lo = pair 0, hi = pair 1, band 1;
//     while !(hi.d > D) and not ([entity+2ECh] >= tick && band >= 3): lo = hi,
//     hi = next pair, band++. I = 00419010(lo.d, float(lo.n), hi.d, float(hi.n), D).
//  5. ratio = (tick - [record+4]) / I (24-bit rounding); s = time when time >= 1.0
//     (COMISS, unordered goes to the remap), else 00419010(10.0, time, 30.0, 1.0, I);
//     s = s * ratio; k = -7 - ftol(s * -8.0); entity+288h = k < 0 ? 0 : min(k, 40h).
//  6. level = (char)entity+288h; level <= 0 -> 0. n = count[level-1] (00F1B050); n >=
//     300h -> 0. Slot (level-1)*300h + n: entity -> 00F57150, band -> 00F1B150, record
//     -> 00F27150; count = n + 1; -> 1.
// x87 runs at the main thread's 24-bit precision control: every product, sum and
// quotient of floats is rounded once to float (float arithmetic here); the int /
// float quotient uses divide_int_by_float_pc24; * -8.0 and * 0.5 are exact.
// ---------------------------------------------------------------------------
bool replication_schedule_sync_0077b280(Raw entity, void** record_arg, std::uint8_t flag,
                                        const std::uint32_t* pairs, ReplicationSyncHost& host) {
    if (record_arg == nullptr) return false;                                               // 0077B28A
    const Raw record{reinterpret_cast<std::uint8_t*>(record_arg)};
    const Raw object{static_cast<std::uint8_t*>(record.ptr(0))};
    if (object.p == nullptr) return false;                                                 // 0077B299
    auto party = [&]() { return *static_cast<const std::uint8_t*>(host.global_address(0x00F87150)); };
    auto tick = [&]() { return *static_cast<const std::int32_t*>(host.global_address(0x00F876B0)); };
    auto pair_d = [&](std::size_t i) { return f32(pairs[2 * i]); };
    auto pair_n = [&](std::size_t i) { return static_cast<std::int32_t>(pairs[2 * i + 1]); };

    const std::uint8_t alive = object.at<std::uint8_t>(0xC8);
    auto& level_byte = entity.at<std::int8_t>(0x288);
    if (flag != 0 && alive == 0) {
        level_byte = 0x40;                                                                 // 0077B2BF
    } else {
        level_byte = 0;
        if (alive != 0 && !host.entity_vslot_5c(entity.p, 6) && flag == 0 &&
            host.entity_vslot_15c(entity.p) != party()) {
            return false;                                                                  // 0077B2FD
        }
    }

    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00E188A8))};
    void* clock = game.ptr(0x2078);
    if (clock == nullptr) clock = game.ptr(0x207C);
    const float time = static_cast<float>(host.clock_vslot_14(clock, object.at<std::uint32_t>(0x50)));  // 0077B326

    std::uint8_t band = 0;                                                                 // BL
    bool to_level = level_byte >= 0x40;                                                    // 0077B331
    if (!to_level && alive != 0 && host.entity_vslot_15c(entity.p) == party()) to_level = true;  // 0077B34F
    if (to_level) {
        if (alive != 0) {                                                                  // 0077B673
            level_byte = 0x40;
            band = 6;
        }
    } else {
        float interval = static_cast<float>(pair_n(0));                                   // 0077B359 CVTSI2SS
        level_byte = 0;
        std::int32_t now;
        if (flag != 0) {
            now = tick();                                                                  // 0077B5C8
        } else {
            const float* const pos = host.entity_world_position_00427eb0(entity.p);
            float v[3] = {pos[0] - object.at<float>(0xAC), pos[1] - object.at<float>(0xB0),
                          pos[2] - object.at<float>(0xB4)};                                // 0077B37E
            float distance = host.vector3_length_0042b2f0(v);                              // 0077B3C0
            if (distance > f32(0x43160000u) &&                                            // 150.0
                host.player_record_has_recent_camera_state_007f7cd0(object.p)) {
                host.vector3_divide_in_place_0042ae80(v, distance);                        // 0077B3F8
                float c4 = object.at<float>(0xC4);                                         // ESP+24h
                const float floor_c4 = f32(0x3ECCCCCDu);                                  // 0.4 (00CE7804)
                const float view_c4 = c4;                                                  // ESP+14h
                const float p1 = object.at<float>(0xBC) * v[1];
                const float p2 = object.at<float>(0xB8) * v[0];
                const float s12 = p1 + p2;
                const float p3 = object.at<float>(0xC0) * v[2];
                const float dot = s12 + p3;                                                // 0077B45A
                const float angle = static_cast<float>(host.stl_inst_007789d0(dot));       // 0077B46E
                const float q = angle / host.max_float_by_ref_00415550(&floor_c4, &view_c4);  // 0077B47B
                const float c2 = f32(0x3F99999Au);                                         // 1.2 (00CE3814)
                float factor;
                if (c2 > q) {                                                              // 0077B498
                    c4 = c4 / f32(0x3F860A92u);                                            // double 00D03DD0 = float(pi/3)
                    c4 = c4 * 0.5f;                                                        // double 00D7A280
                    factor = host.interpolate_clamped_00419010(f32(0x3F000000u), c4, c2, c2, q);
                } else {
                    factor = host.interpolate_clamped_00419010(c2, c2, f32(0x40400000u), f32(0x40C00000u), q);
                }
                distance = factor * distance;                                              // 0077B4F2
            }
            now = tick();                                                                  // 0077B4F6
            const bool capped = entity.at<std::int32_t>(0x2EC) >= now;                     // SETGE DL
            float lo_d = pair_d(0);
            float lo_n = static_cast<float>(pair_n(0));
            float hi_d = pair_d(1);
            float hi_n = static_cast<float>(pair_n(1));
            band = 1;
            if (!(hi_d > distance)) {                                                      // 0077B548
                std::size_t i = 1;
                while (!(capped && band >= 3)) {                                           // 0077B54E
                    const float next_d = pair_d(i + 1);
                    const float next_n = static_cast<float>(pair_n(i + 1));
                    ++i;
                    ++band;
                    lo_d = hi_d;
                    lo_n = hi_n;
                    hi_d = next_d;
                    hi_n = next_n;
                    if (hi_d > distance) break;                                            // 0077B587
                }
            }
            interval = host.interpolate_clamped_00419010(lo_d, lo_n, hi_d, hi_n, distance);  // 0077B5BD
        }
        const std::int32_t elapsed =
            static_cast<std::int32_t>(static_cast<std::uint32_t>(now) - record.at<std::uint32_t>(4));
        const float ratio = divide_int_by_float_pc24(elapsed, interval);                   // 0077B5EC
        float scale;
        if (time >= f32(0x3F800000u)) {                                                    // COMISS 1.0
            scale = time;
        } else {
            scale = host.interpolate_clamped_00419010(f32(0x41200000u), time, f32(0x41F00000u), 1.0f, interval);
        }
        scale = scale * ratio;                                                             // 0077B635
        const std::int32_t t = host.ftol_00bf7420(static_cast<double>(scale) * -8.0);      // 0077B647, exact
        const std::uint32_t k = 0xFFFFFFF9u - static_cast<std::uint32_t>(t);               // -7 - t
        if ((k & 0x80000000u) != 0) {
            level_byte = 0;                                                                // 0077B657
        } else {
            level_byte = static_cast<std::int8_t>(static_cast<std::int32_t>(k) > 0x40 ? 0x40 : static_cast<std::int32_t>(k));
        }
    }

    const std::int8_t level = level_byte;                                                  // 0077B685
    if (level <= 0) return false;
    auto* const counts = static_cast<std::int32_t*>(host.global_address(0x00F1B050));
    const std::int32_t n = counts[level - 1];
    if (n >= 0x300) return false;                                                          // 0077B6A2
    const std::size_t slot = static_cast<std::size_t>(level - 1) * 0x300 + static_cast<std::size_t>(n);
    static_cast<void**>(host.global_address(0x00F57150))[slot] = entity.p;
    static_cast<std::uint8_t*>(host.global_address(0x00F1B150))[slot] = band;
    counts[level - 1] = n + 1;
    static_cast<void**>(host.global_address(0x00F27150))[slot] = record_arg;
    return true;
}

// ---------------------------------------------------------------------------
// 00B68D70, __thiscall(ECX = LuaObject; out (sret), const NativeString* path, bool* ok)
// RET 0Ch -> EAX = out; EH frame 00CC15E9. Assembly 00B68D70-00B6911E:
//  - out = copy of this (00B66FA0); rest = copy of path; *ok = 1; nothing more when
//    rest is empty.
//  - Per pass: segment = rest up to the first '.' (strcspn; split only when the
//    position is neither rest.len nor negative), rest = text after it (two 00469840
//    substrings, each assigned through 0041DD40 + memcpy and released); with no dot
//    the whole rest moves into segment and rest is released and zeroed (for an empty
//    rest the image copies 0 bytes of "" into rest.data instead).
//  - Empty segment: *ok = 0, its block returned with size 1, stop.
//  - out unbound (kind 0) or a kind-2 non-table (lua_type != 5): *ok = 0, keep going.
//    Otherwise tmp = out[segment] (00B68100); when tmp is a kind-2 nil and atol(segment)
//    is non-zero or segment is "0" (_stricmp), tmp = out[n] (00B67720 + 00B67690, the
//    temporary's tracked release 00B66DE0 when bound); out = tmp (00B67690) and tmp's
//    tracked release.
//  - Segment released; repeat while rest.len != 0; rest released.
// ---------------------------------------------------------------------------
namespace {

void release_ns(const NativeStringImage& s, std::uint32_t bytes, LuaDotPathHost& host) {
    if (s.data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, s.data, bytes, 1);
    }
}

// The inlined string assignment: when distinct, resize then memcpy(dest.len) if src.len.
void assign_ns(NativeStringImage& dest, const NativeStringImage* src, LuaDotPathHost& host) {
    if (&dest == src) return;
    host.native_string_resize_0041dd40(&dest, src->length, 1);
    if (src->length != 0) host.memcpy_00bf7680(dest.data, src->data, dest.length);
}

std::int32_t lua_kind(LuaObjectStorage* o) { return Raw{o->bytes}.at<std::int32_t>(4); }

std::int32_t lua_type_of(LuaObjectStorage* o, LuaDotPathHost& host) {
    const Raw r{o->bytes};
    return host.lua_type_00a675a0(Raw{static_cast<std::uint8_t*>(r.ptr(0))}.ptr(4), r.at<std::int32_t>(8));
}

// The inlined LuaObject destructor: tracked release when bound.
void destroy_tracked(LuaObjectStorage* o, LuaDotPathHost& host) {
    const Raw r{o->bytes};
    if (r.at<std::int32_t>(4) != 0) {
        host.lua_state_owner_release_tracked_00b66de0(r.ptr(0), o, r.at<std::int32_t>(8), 1);
    }
}

}  // namespace

LuaObjectStorage* lua_object_resolve_dot_path_00b68d70(LuaObjectStorage* self, LuaObjectStorage* out,
                                                       const NativeStringImage* path, std::uint8_t* ok,
                                                       LuaDotPathHost& host) {
    host.lua_object_copy_construct_00b66fa0(out, self);                                    // 00B68D97
    NativeStringImage rest{0, nullptr};                                                    // ESP+14h
    assign_ns(rest, path, host);
    *ok = 1;                                                                               // 00B68DFA
    if (rest.length != 0) {
        do {
            NativeStringImage segment{0, nullptr};                                         // ESP+1Ch
            bool split = false;
            if (rest.data != nullptr && rest.length != 0) {
                const auto pos = static_cast<std::int32_t>(host.strcspn_00bf9a50(rest.data, "."));
                if (pos != static_cast<std::int32_t>(rest.length) && pos >= 0) {           // 00B68E36
                    NativeStringImage t1, t2;                                              // ESP+24h, ESP+2Ch
                    assign_ns(segment, host.native_string_substring_00469840(&rest, &t1, 0,
                                                                             static_cast<std::uint32_t>(pos)), host);
                    release_ns(t1, t1.length + 1, host);
                    assign_ns(rest, host.native_string_substring_00469840(&rest, &t2,
                                                                          static_cast<std::uint32_t>(pos + 1),
                                                                          0x7FFFFFFFu), host);
                    release_ns(t2, t2.length + 1, host);                                   // 00B68F1C
                    split = true;
                }
            }
            if (!split) {
                const std::uint32_t length = rest.length;                                  // 00B69030
                host.native_string_resize_0041dd40(&segment, length, 1);
                if (length != 0) {
                    host.memcpy_00bf7680(segment.data, rest.data, segment.length);         // 00B6904F
                    release_ns(rest, length + 1, host);
                    rest = {0, nullptr};
                } else if (rest.data != nullptr) {
                    host.memcpy_00bf7680(rest.data, "", 0);                                // 00B69089
                }
            }
            if (segment.length == 0) {                                                     // 00B68F2A
                *ok = 0;
                release_ns(segment, 1, host);                                              // 00B690E7
                break;
            }
            const std::int32_t kind = lua_kind(out);
            if (kind == 0 || (kind == 2 && lua_type_of(out, host) != 5)) {
                *ok = 0;                                                                   // 00B6909E
            } else {
                LuaObjectStorage tmp;                                                      // ESP+34h
                host.lua_object_get_by_native_string_00b68100(out, &tmp, &segment);        // 00B68F66
                if (lua_kind(&tmp) == 2 && lua_type_of(&tmp, host) == 0) {
                    const char* const text = segment.data != nullptr
                                                 ? segment.data
                                                 : static_cast<const char*>(host.global_address(0x0108FF2C));
                    const std::int32_t n = host.atol_00bf8417(text);                       // 00B68FA0
                    if (n != 0 || (segment.data != nullptr && host.stricmp_00bf7fbf(segment.data, "0") == 0)) {
                        LuaObjectStorage tmp2;                                             // ESP+48h
                        host.lua_object_assign_00b67690(&tmp, host.lua_object_get_by_index_00b67720(out, &tmp2, n));
                        destroy_tracked(&tmp2, host);                                      // 00B68FF9
                    }
                }
                host.lua_object_assign_00b67690(out, &tmp);                                // 00B69009
                destroy_tracked(&tmp, host);                                               // 00B69029
            }
            release_ns(segment, segment.length + 1, host);                                 // 00B690BC
        } while (rest.length != 0);
    }
    release_ns(rest, rest.length + 1, host);                                               // 00B69103
    return out;
}

}  // namespace bsp::cc13_front8
