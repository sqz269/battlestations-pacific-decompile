// Packet cc13-seg47b (cc13_seg47b marker). See include/bsp/cc13_seg47b.hpp and
// docs/CC13_SEG47B.md. Each function carries its address, original ABI and
// the simplifications it makes. Reconstructed and build-tested only.

#include "bsp/cc13_seg47b.hpp"
#include "bsp/unit_rudder.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_seg47b {

namespace {

// The flight-state request message every 007C1570-family sender builds on
// its stack: 0075B430(C3h) base (24h bytes), +18h word 0, +1Ah byte 0, +4 = 1,
// +1Ch byte 0, vptr 00D03504, +20h = the current state's low byte, +21h = the
// requested state; routed through 0077C2A0(plane; &message, 7, 0). No
// destructor runs after the route (the EH state is only set to 0).
void send_flight_state_message(Seg47bHost& host, Raw plane, std::uint8_t current, std::uint8_t requested) {
    alignas(4) std::uint8_t storage[0x24];
    Raw message{storage};
    host.session_message_construct_0075b430(message, 0xC3);
    message.at<std::uint16_t>(0x18) = 0;
    message.at<std::uint8_t>(0x1A) = 0;
    message.at<std::uint32_t>(4) = 1;
    message.at<std::uint8_t>(0x1C) = 0;
    message.at<std::uint32_t>(0) = 0x00D03504u;
    message.at<std::uint8_t>(0x20) = current;
    message.at<std::uint8_t>(0x21) = requested;
    host.session_route_message_0077c2a0(plane, storage, 7, 0);
}

bool session_is_client(Seg47bHost& host) {
    return raw(host.global_pointer(0x00E188A8u)).at<std::int32_t>(0x1FE4) == 2;
}

bool state_in_7645(std::int32_t s) { return s == 7 || s == 6 || s == 4 || s == 5; }

// The inline `if (!entity+C8h) 00414DB0(entity)` pose refresh.
void refresh_pose(Seg47bHost& host, Raw entity) {
    if (entity.at<std::uint8_t>(0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
}

// The inline length idiom: s = (float)((dx*dx + dy*dy) + dz*dz) in x87 (here
// double: the float products are exact, the two sums may round differently
// from 80-bit), then (double)s > 1e-10 (00CE3820, FCOMI/JBE) -> (float)sqrt(s)
// (CRT _CIsqrt), else 0.0f.
float length_or_zero(float dx, float dy, float dz) {
    const float s = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy +
                                       static_cast<double>(dz) * dz);
    if (static_cast<double>(s) > 1e-10) return static_cast<float>(std::sqrt(static_cast<double>(s)));
    return 0.0f;
}

// The MSVC checked-iterator test `it < end` (else _invalid_parameter_noinfo).
void check_before_end(Seg47bHost& host, const std::uint8_t* it, const std::uint8_t* end) {
    if (!(it < end)) host.invalid_parameter_00bf6713();
}

// The inline NativeString {length +0, data +4} destructor: when data is set,
// 00BD1510(00419CC0(); data, length + 1, 1).
void native_string_release(Seg47bHost& host, Raw string) {
    void* const data = string.ptr(4);
    if (data == nullptr) return;
    host.sized_pool_return_block_00bd1510(host.sized_pool_00419cc0(), data,
                                          string.at<std::uint32_t>(0) + 1, 1);
}

// The gun-effect pop loop shared by 007CA5F0 (not firing) and 007CC580: while
// the count +A34h is non-zero, the last effect of data +A30h is 00867B10'd and
// its byte +9 set to 1; then, with the count re-read and non-zero, the last
// slot is released (InterlockedDecrement(+4) == 0 -> vtable[0]) and zeroed
// when set, and the count drops by one.
void stop_and_pop_gun_effects(Seg47bHost& host, Raw plane) {
    while (plane.at<std::int32_t>(0xA34) != 0) {
        const std::int32_t count = plane.at<std::int32_t>(0xA34);
        const Raw last = raw(static_cast<void**>(plane.ptr(0xA30))[count - 1]);
        host.point_effect_stop_children_00867b10(last);
        last.at<std::uint8_t>(9) = 1;
        const std::int32_t now = plane.at<std::int32_t>(0xA34);
        if (now == 0) continue;
        void** const slot = &static_cast<void**>(plane.ptr(0xA30))[now - 1];
        if (*slot != nullptr) {
            const Raw p = raw(*slot);
            if (host.interlocked_decrement_ce2220(&p.at<long>(4)) == 0) host.refcounted_vcall_destroy_00(p);
            *slot = nullptr;
        }
        plane.at<std::int32_t>(0xA34) = now - 1;
    }
}

}  // namespace

// 007C1570, __thiscall(ECX = plane; requested, force), RET 8, SEH frame.
// cur = +900h. Return when cur == 2, requested == 4 and session mode
// [[00E188A8]+1FE4h] == 2; return when !force and cur == requested. Mode != 2:
// the C3h message (cur's low byte at +20h, requested at +21h; the image parks
// cur's byte in the dead `force` argument slot, 007C15D6). Mode 2: 007C1430
// directly when requested == 3, or when cur is 7/6/4/5 and requested is
// 7/4/5/6; otherwise nothing.
void plane_request_flight_state_007c1570(Seg47bHost& host, Raw plane, std::int32_t requested, bool force) {
    const std::int32_t cur = plane.at<std::int32_t>(0x900);
    const bool client = session_is_client(host);
    if (cur == 2 && requested == 4 && client) return;
    if (!force && cur == requested) return;
    if (!client) {
        send_flight_state_message(host, plane, static_cast<std::uint8_t>(cur),
                                  static_cast<std::uint8_t>(requested));
        return;
    }
    if (requested == 3 ||
        (state_in_7645(cur) && (requested == 7 || requested == 4 || requested == 5 || requested == 6))) {
        host.plane_set_flight_state_007c1430(plane, requested);
    }
}

// 007C17D0, __thiscall(ECX = plane), RET 0, SEH frame. Only when byte +904h
// is set: cur = +900h; return when cur == 2 in session mode 2. Mode != 2: the
// C3h message (cur, 4). Mode 2: 007C1430(plane, 4) when cur is 7/6/4/5.
void plane_notify_takeoff_start_007c17d0(Seg47bHost& host, Raw plane) {
    if (plane.at<std::uint8_t>(0x904) == 0) return;
    const std::int32_t cur = plane.at<std::int32_t>(0x900);
    const bool client = session_is_client(host);
    if (cur == 2 && client) return;
    if (!client) {
        send_flight_state_message(host, plane, static_cast<std::uint8_t>(cur), 4);
        return;
    }
    if (state_in_7645(cur)) host.plane_set_flight_state_007c1430(plane, 4);
}

// 007C18B0, __thiscall(ECX = plane; float dt), RET 4: the attitude angles and
// their rates from the pose matrix +CCh and the velocities.
// Old heading +C6Ch, pitch +C64h and bank +C68h are read first, then the pose
// refresh. f = matrix row 2 (+ECh..+F4h): h = (float)sqrt((float)(fx*fx) +
// (float)(fz*fz)) (no zero test); +C64h = (float)atan2(fy, h).
// v = 004F9B30(&local; row 2, 00F8758C); s = (float)((v0*v0 + v1*v1) + v2*v2);
// when s > 1e-10 (double 00CE3820) and L = (float)sqrt(s) > 0.001f (00D7A23C,
// COMISS/JBE): 0085E4D0(&M, +CCh; v0/L, v1/L, v2/L, new pitch) (the quotients
// rounded to float); +C6Ch = (float)atan2(M[28h], M[20h]); M = M * rotY(that
// heading) (00B646E0 on a copy, 004134F0, 00413920, 004134F0; the image
// negates the heading twice before 00B646E0, a no-op); +C68h =
// (float)atan2(M[18h], M[14h]); +C6Ch = 00438B10(1.5707964f (00CE3C64),
// heading); with dt > 0, d = 00438B10(new +C6Ch, old heading) and +C70h =
// (float)(d / dt) only when 0.5f (00CE3800) > |d|. With dt > 0 (also when the
// heading block was skipped): +C74h = 00438B10(+C64h, old pitch) / dt, +C78h =
// 00438B10(+C68h, old bank) / dt.
// Velocities: S = (float)((vy^2 + vx^2) + vz^2) of +AC8h..+AD0h, speed =
// S > 1e-10 ? (float)sqrt(S) : 0; when speed > 0.1 (double 00D7A3A0):
// +C7Ch = (float)atan2(vy, |(vx, vz)|) and +C84h = (float)atan2(+AE4h,
// |(+AE0h, +AE8h)|), each length with the same 1e-10 test. With dt > 0:
// +C80h = 00438B10(+C7Ch, old +C7Ch) / dt and +C88h = 00438B10(+C84h,
// old +C84h) / dt (the old values read before the velocity block).
// Simplifications: x87 sums of squares are formed in double (the float
// products are exact; the sums may round differently from 80-bit); CRT
// atan2 / sqrt are std:: on doubles of the stored floats (the image passes
// the CRT the unrounded 80-bit atan2 inputs only where they are float loads,
// so this matches); 00438B10 is bsp::wrapped_angle_subtract_00438b10.
void plane_update_pose_derived_state_007c18b0(Seg47bHost& host, Raw plane, float dt) {
    const float old_heading = plane.at<float>(0xC6C);
    const float old_pitch = plane.at<float>(0xC64);
    const float old_bank = plane.at<float>(0xC68);
    refresh_pose(host, plane);
    const float fx = plane.at<float>(0xCC + 0x20);
    const float fz = plane.at<float>(0xCC + 0x28);
    const float h = static_cast<float>(std::sqrt(static_cast<double>(fx * fx + fz * fz)));
    const float fy = plane.at<float>(0xCC + 0x24);
    const float pitch = static_cast<float>(std::atan2(static_cast<double>(fy), static_cast<double>(h)));
    plane.at<float>(0xC64) = pitch;
    float v_storage[3];
    const float* const vp = host.vector3_cross_004f9b30(
        v_storage, &plane.at<float>(0xCC + 0x20), static_cast<const float*>(host.global_address(0x00F8758Cu)));
    const float v[3] = {vp[0], vp[1], vp[2]};
    const float s = static_cast<float>(static_cast<double>(v[0]) * v[0] + static_cast<double>(v[1]) * v[1] +
                                       static_cast<double>(v[2]) * v[2]);
    bool heading_block = false;
    if (static_cast<double>(s) > 1e-10) {
        const float length = static_cast<float>(std::sqrt(static_cast<double>(s)));
        if (length > 0.001f) heading_block = true;
        if (heading_block) {
            alignas(4) std::uint8_t m[0x40];
            host.matrix_rotate_by_angular_velocity_0085e4d0(m, plane.field(0xCC), v[0] / length, v[1] / length,
                                                            v[2] / length, pitch);
            const Raw mr{m};
            float heading = static_cast<float>(
                std::atan2(static_cast<double>(mr.at<float>(0x28)), static_cast<double>(mr.at<float>(0x20))));
            plane.at<float>(0xC6C) = heading;
            alignas(4) std::uint8_t rotation[0x40];
            alignas(4) std::uint8_t copy[0x40];
            alignas(4) std::uint8_t product[0x40];
            void* const built = host.matrix_build_rotation_y_00b646e0(rotation, &heading);
            host.matrix_assign_004134f0(Raw{copy}, built);
            void* const multiplied = host.matrix_multiply_00413920(mr, product, copy);
            host.matrix_assign_004134f0(mr, multiplied);
            plane.at<float>(0xC68) = static_cast<float>(
                std::atan2(static_cast<double>(mr.at<float>(0x18)), static_cast<double>(mr.at<float>(0x14))));
            const float heading2 = bsp::wrapped_angle_subtract_00438b10(1.5707963705062866f, plane.at<float>(0xC6C));
            plane.at<float>(0xC6C) = heading2;
            if (!(dt > 0.0f)) goto velocity_block;
            const float d = bsp::wrapped_angle_subtract_00438b10(heading2, old_heading);
            if (0.5f > std::fabs(d)) plane.at<float>(0xC70) = d / dt;
        }
    }
    if (dt > 0.0f) {
        plane.at<float>(0xC74) = bsp::wrapped_angle_subtract_00438b10(plane.at<float>(0xC64), old_pitch) / dt;
        plane.at<float>(0xC78) = bsp::wrapped_angle_subtract_00438b10(plane.at<float>(0xC68), old_bank) / dt;
    }
velocity_block : {
    const float old_track = plane.at<float>(0xC7C);
    const float old_aoa = plane.at<float>(0xC84);
    const float vx = plane.at<float>(0xAC8);
    const float vy = plane.at<float>(0xACC);
    const float vz = plane.at<float>(0xAD0);
    const double vx2 = static_cast<double>(vx) * vx;
    const double vz2 = static_cast<double>(vz) * vz;
    const float sum = static_cast<float>(static_cast<double>(vy) * vy + vx2 + vz2);
    float speed = 0.0f;
    if (static_cast<double>(sum) > 1e-10) speed = static_cast<float>(std::sqrt(static_cast<double>(sum)));
    if (static_cast<double>(speed) > 0.10000000149011612) {
        const float hs = static_cast<float>(vx2 + vz2);
        float hl = 0.0f;
        if (static_cast<double>(hs) > 1e-10) hl = static_cast<float>(std::sqrt(static_cast<double>(hs)));
        plane.at<float>(0xC7C) =
            static_cast<float>(std::atan2(static_cast<double>(vy), static_cast<double>(hl)));
        const float a = plane.at<float>(0xAE0);
        const float b = plane.at<float>(0xAE4);
        const float c = plane.at<float>(0xAE8);
        const float ls = static_cast<float>(static_cast<double>(a) * a + static_cast<double>(c) * c);
        float l2 = 0.0f;
        if (static_cast<double>(ls) > 1e-10) l2 = static_cast<float>(std::sqrt(static_cast<double>(ls)));
        plane.at<float>(0xC84) = static_cast<float>(std::atan2(static_cast<double>(b), static_cast<double>(l2)));
    }
    if (dt > 0.0f) {
        plane.at<float>(0xC80) = bsp::wrapped_angle_subtract_00438b10(plane.at<float>(0xC7C), old_track) / dt;
        plane.at<float>(0xC88) = bsp::wrapped_angle_subtract_00438b10(plane.at<float>(0xC84), old_aoa) / dt;
    }
}
}

// 007C2090, __thiscall(ECX = plane), RET 0, SEH frame: when +900h != 2 and the
// session mode is not 2, the C3h message (byte +900h, 2). Nothing otherwise.
void plane_request_flight_state_two_007c2090(Seg47bHost& host, Raw plane) {
    if (plane.at<std::int32_t>(0x900) == 2) return;
    if (session_is_client(host)) return;
    send_flight_state_message(host, plane, plane.at<std::uint8_t>(0x900), 2);
}

// 007C2130, __thiscall(ECX = plane), RET 0, SEH frame: 007C2090 with state 1.
void plane_request_flight_state_one_007c2130(Seg47bHost& host, Raw plane) {
    if (plane.at<std::int32_t>(0x900) == 1) return;
    if (session_is_client(host)) return;
    send_flight_state_message(host, plane, plane.at<std::uint8_t>(0x900), 1);
}

// 007C2AF0, __thiscall(ECX = record; plane, owner), RET 8.
// When record+9Dh is set, the record is filled from the plane first: +38h..+4Ch
// = plane+9E4h..+9F8h (six dwords), flags +A1h..+A5h = 1, +2Ch..+34h =
// plane+6A4h..+6ACh, 0042D2E0(plane+674h; +20h, +24h, +28h), flags
// +A6h..+A8h = 1, 007BAA80(record+50h; 007D8330(plane+AB0h; local 3Ch)),
// flags +A9h..+AEh = 1. Then +1Ch = 007C2810(plane; owner); when non-null each
// field is copied record -> state when its flag is set and state -> record
// otherwise; the six +5Ch..+70h fields only when +A0h is set. The FLD/FSTP
// float copies are bitwise here (an sNaN would be quieted by the image).
void plane_state_record_serialize_007c2af0(Seg47bHost& host, Raw record, Raw plane, void* owner) {
    if (record.at<std::uint8_t>(0x9D) != 0) {
        for (int i = 0; i < 6; ++i) {
            record.at<std::uint32_t>(0x38 + 4 * i) = plane.at<std::uint32_t>(0x9E4 + 4 * i);
        }
        record.at<std::uint8_t>(0xA5) = 1;
        record.at<std::uint8_t>(0xA4) = 1;
        record.at<std::uint8_t>(0xA3) = 1;
        record.at<std::uint8_t>(0xA1) = 1;
        record.at<std::uint8_t>(0xA2) = 1;
        record.at<float>(0x2C) = plane.at<float>(0x6A4);
        record.at<float>(0x30) = plane.at<float>(0x6A8);
        record.at<float>(0x34) = plane.at<float>(0x6AC);
        host.matrix_extract_normalized_basis_angles_0042d2e0(plane.field(0x674), &record.at<float>(0x20),
                                                             &record.at<float>(0x24), &record.at<float>(0x28));
        alignas(4) std::uint8_t local[0x3C];
        record.at<std::uint8_t>(0xA8) = 1;
        record.at<std::uint8_t>(0xA7) = 1;
        record.at<std::uint8_t>(0xA6) = 1;
        void* const filled = host.flight_controller_write_state_record_007d8330(plane.sub(0xAB0), local);
        host.plane_state_record_copy_007baa80(record.sub(0x50), filled);
        record.at<std::uint8_t>(0xAB) = 1;
        record.at<std::uint8_t>(0xAA) = 1;
        record.at<std::uint8_t>(0xA9) = 1;
        record.at<std::uint8_t>(0xAE) = 1;
        record.at<std::uint8_t>(0xAD) = 1;
        record.at<std::uint8_t>(0xAC) = 1;
    }
    void* const state_ptr = host.plane_state_record_get_or_create_007c2810(plane, owner);
    record.ptr(0x1C) = state_ptr;
    if (state_ptr == nullptr) return;
    struct Pair {
        std::uint16_t flag, rec, st;
    };
    const auto sync = [&](const Pair& p) {
        const Raw st = record.deref(0x1C);
        if (record.at<std::uint8_t>(p.flag) != 0) {
            st.at<float>(p.st) = record.at<float>(p.rec);
        } else {
            record.at<float>(p.rec) = st.at<float>(p.st);
        }
    };
    static constexpr Pair kBase[] = {{0xA2, 0x48, 0x10}, {0xA1, 0x44, 0x0C}, {0xA3, 0x38, 0x00},
                                     {0xA4, 0x3C, 0x04}, {0xA5, 0x40, 0x08}, {0xA6, 0x20, 0x48},
                                     {0xA7, 0x24, 0x4C}, {0xA8, 0x28, 0x50}};
    for (const Pair& p : kBase) sync(p);
    if (record.at<std::uint8_t>(0xA0) == 0) return;
    static constexpr Pair kExtra[] = {{0xA9, 0x5C, 0x60}, {0xAA, 0x60, 0x64}, {0xAB, 0x64, 0x68},
                                      {0xAC, 0x68, 0x78}, {0xAD, 0x6C, 0x7C}, {0xAE, 0x70, 0x80}};
    for (const Pair& p : kExtra) sync(p);
}

// 007C21D0, __thiscall(ECX = plane; out float[3]), RET 4, EAX = out: the HUD
// bomb-sight impact point (callers 006082D0, 00609BD0). After the pose
// refresh, P = +FCh..+104h, y5 = (float)(P.y - 5.0) (double 00D7A370), g = 0.
// Up to 11 passes: dy = P.y - g; t = 0 unless 0 < dy (FCOMI/JB, so a NaN dy
// falls too): a = vtable[34h]().y - [00E08E54], t = (float)((sqrt((float)(2 dy
// x 9.81 + a^2)) + a) / 9.81) (double 00CF9058); t' = 0.05f (00D0DE84) + t;
// v = vtable[34h]() again; out.y = g; out.x = (float)(v.x t') + P.x, out.z =
// (float)(t' v.z) + P.z; G = 007BA340([00E188A8]; out.x, out.z), clamped to 0
// below and to cap = (0 > y5 ? 0 : y5) above (a NaN G stays G); stop when 5.0f
// (00CE3850) > |G' - g| (g kept), else g = G' and stop after the eleventh pass.
// Then w = 00419010(0, 0.8f (00CE74F8), 0.5f (00CE3800), 0, [00F876A4] -
// +C5Ch) and, when w > 0, out.y = 00419010(0, out.y, 1.0f, +C60h, w); +C60h =
// out.y, +C5Ch = [00F876A4]. The image keeps a 0.0 on the x87 stack across the
// passes and feeds it to both 00419010 calls as x0 (and y1 of the first).
// Simplification: the 2 dy g + a^2 and (r + a) / g terms are formed in double.
float* plane_predict_bomb_impact_point_007c21d0(Seg47bHost& host, Raw plane, float* out) {
    refresh_pose(host, plane);
    const float px = plane.at<float>(0xFC);
    const float py = plane.at<float>(0x100);
    const float pz = plane.at<float>(0x104);
    const float y5 = static_cast<float>(static_cast<double>(py) - 5.0);
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    float g = 0.0f;
    for (std::int32_t pass = 1;; ++pass) {
        const float dy = py - g;
        float t = 0.0f;
        if (!(0.0f >= dy)) {
            float scratch[3];
            const float* const v = host.plane_vcall_world_velocity_34(plane, scratch);
            const float a = v[1] - *static_cast<const float*>(host.global_address(0x00E08E54u));
            const float s = static_cast<float>((static_cast<double>(dy) + dy) * 9.8100004196167 +
                                               static_cast<double>(a) * a);
            const float r = static_cast<float>(std::sqrt(static_cast<double>(s)));
            t = static_cast<float>((static_cast<double>(r) + a) / 9.8100004196167);
        }
        const float tp = 0.05f + t;
        float scratch2[3];
        const float* const v = host.plane_vcall_world_velocity_34(plane, scratch2);
        out[1] = g;
        const float dx = v[0] * tp;
        const float dz = tp * v[2];
        const float qx = dx + px;
        const float qz = dz + pz;
        out[0] = qx;
        out[2] = qz;
        const float cap = 0.0f > y5 ? 0.0f : y5;
        const float ground = host.plane_landscape_height_at_007ba340(game, qx, qz);
        float next;
        if (0.0f > ground) {
            next = 0.0f;
        } else if (ground > cap) {
            next = cap;
        } else {
            next = ground;
        }
        if (5.0f > std::fabs(next - g)) break;
        g = next;
        if (pass > 10) break;
    }
    const float now = *static_cast<const float*>(host.global_address(0x00F876A4u));
    const float elapsed = now - plane.at<float>(0xC5C);
    const float w = host.math_interpolate_clamped_00419010(0.0f, 0.8f, 0.5f, 0.0f, elapsed);
    if (w > 0.0f) out[1] = host.math_interpolate_clamped_00419010(0.0f, out[1], 1.0f, plane.at<float>(0xC60), w);
    plane.at<float>(0xC60) = out[1];
    plane.at<float>(0xC5C) = now;
    return out;
}

// 007C2450, __thiscall(ECX = plane; out float[3]), RET 4, EAX = out, SEH frame:
// the HUD rocket impact point. After the pose refresh: alt = (float)(P.y -
// 007BD120(plane)) (the subtraction against the double of P.y), and the start
// S = (P.x, alt, P.z); after a second refresh, out = S + (float)(row2 x
// 10000.0) (double 00CE4BD8) component by component; a hit record is reset
// (00470470) and 00904400([[00E188A8]+19CCh]; 44h, &S, out, &record, 0); on a
// hit out = record+8..+10. When alt > 0 and 0 > out.y (COMISS, NaN skips):
// out.y = 0, out.x = P.x + (out.x - P.x) alt / (alt - oy), out.z = P.z +
// alt (out.z - P.z) / (alt - oy) (x87, here double, rounded to float once);
// the record is destroyed (004704B0).
float* plane_predict_rocket_impact_point_007c2450(Seg47bHost& host, Raw plane, float* out) {
    refresh_pose(host, plane);
    const float px = plane.at<float>(0xFC);
    const float py = plane.at<float>(0x100);
    const float pz = plane.at<float>(0x104);
    const double py_d = py;
    const float alt = static_cast<float>(py_d - static_cast<double>(host.rocket_ignition_lift_007bd120(plane)));
    const float start[3] = {px, alt, pz};
    refresh_pose(host, plane);
    const float fx = static_cast<float>(static_cast<double>(plane.at<float>(0xEC)) * 10000.0);
    const float fy = static_cast<float>(static_cast<double>(plane.at<float>(0xF0)) * 10000.0);
    const float fz = static_cast<float>(10000.0 * static_cast<double>(plane.at<float>(0xF4)));
    out[0] = fx + px;
    out[1] = fy + alt;
    out[2] = fz + pz;
    alignas(4) std::uint8_t record[0x54];
    host.hit_record_reset_00470470(record);
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    if (host.world_segment_query_00904400(game.deref(0x19CC), 0x44, start, out, record, 0)) {
        const Raw r{record};
        out[0] = r.at<float>(8);
        out[1] = r.at<float>(0xC);
        out[2] = r.at<float>(0x10);
    }
    if (alt > 0.0f) {
        const float oy = out[1];
        if (0.0f > oy) {
            out[1] = 0.0f;
            const double drop = static_cast<double>(alt) - oy;
            out[0] = static_cast<float>((static_cast<double>(out[0]) - px) * alt / drop + px);
            out[2] = static_cast<float>(static_cast<double>(alt) * (static_cast<double>(out[2]) - pz) / drop + pz);
        }
    }
    host.hit_record_destroy_004704b0(record);
    return out;
}

// 007C4850, __thiscall(ECX = class), AL. Refusals (004254B0 message, AL = 0):
// +20Ch <= 0 (signed) "KamikazeBulletClass", +15Ch == 0.0f (UCOMISS/JNP)
// "BombControlLimit == 0", 0.0f >= +1F0h (COMISS/JB; NaN passes) "DropAngle
// <= 0". Defaults when zero: +170h, +174h = 1.0f; +178h = 0.5235988f
// (00CEC724); +1BCh / +1C0h / +1C4h = (float)(+1A8h / +1ACh / +1B0h x 2.0).
// q = (float)(+164h / m / m) with m = +188h -> +50Ch; +504h = (float)sqrt(
// (float)(9.81 / (q (+1DCh + 1.0)))); +520h = +524h = 1.0f; +510h / +514h =
// sin(+1CCh / +1D0h); +518h = 00412E20(+1F0h), +51Ch = 00412E20((float)(+1F0h
// x 0.7)); +528h..+530h = (0, cos, sin)(+200h); +534h..+53Ch = (0, cos,
// sin)(+204h); +4FCh = 0; +27Ch = identity (004134F0); +2ACh = 0, +2B0h =
// 1.3f (00CEB4B4), +2B4h = 3.0f; +1E4h = 007D98F0((float)(tuning+24Ch x
// +184h)), +1E8h = 007D98F0(+18Ch), +1ECh = (float)(+1E4h x 0.6); r =
// 007DB4D0() -> +26Ch, +268h = 00415550(tuning+54Ch, (float)(r x 1.1));
// +19Ch = 00415510(+19Ch, (float)(007C47F0() x (+198h == 0 ? 2.2 : 1.5)));
// +1A0h = (float)(+164h / (v v 1.6)) with v = (float)(tuning+24Ch x +184h);
// with b = [+50h]+28h (the model bounds): +508h = -b[1] - +194h; +500h =
// max(max(-b[0], b[3]), max(-b[1], b[4]), max(-b[2], b[5])) through 00415550
// in the image's order; then the tail jump 00951F20(class), whose AL is
// returned. Simplification: extended-precision quotients formed in double.
bool plane_class_derive_flight_constants_007c4850(Seg47bHost& host, Raw cls) {
    if (!(cls.at<std::int32_t>(0x20C) > 0)) {
        host.error_log_004254b0("ERROR : 'KamikazeBulletClass' wrong/undefined!");
        return false;
    }
    if (cls.at<float>(0x15C) == 0.0f) {
        host.error_log_004254b0("ERROR : BombControlLimit == 0");
        return false;
    }
    if (0.0f >= cls.at<float>(0x1F0)) {
        host.error_log_004254b0("ERROR : DropAngle <= 0");
        return false;
    }
    if (cls.at<float>(0x170) == 0.0f) cls.at<float>(0x170) = 1.0f;
    if (cls.at<float>(0x174) == 0.0f) cls.at<float>(0x174) = 1.0f;
    if (cls.at<float>(0x178) == 0.0f) cls.at<float>(0x178) = 0.5235987901687622f;
    if (cls.at<float>(0x1BC) == 0.0f) cls.at<float>(0x1BC) = static_cast<float>(cls.at<float>(0x1A8) * 2.0);
    if (cls.at<float>(0x1C0) == 0.0f) cls.at<float>(0x1C0) = static_cast<float>(cls.at<float>(0x1AC) * 2.0);
    if (cls.at<float>(0x1C4) == 0.0f) cls.at<float>(0x1C4) = static_cast<float>(2.0 * cls.at<float>(0x1B0));
    const float m = cls.at<float>(0x188);
    const float q = static_cast<float>(static_cast<double>(cls.at<float>(0x164)) / m / m);
    cls.at<float>(0x50C) = q;
    const float inner =
        static_cast<float>(9.8100004196167 / (static_cast<double>(q) * (static_cast<double>(cls.at<float>(0x1DC)) + 1.0)));
    cls.at<float>(0x524) = 1.0f;
    cls.at<float>(0x504) = static_cast<float>(std::sqrt(static_cast<double>(inner)));
    cls.at<float>(0x520) = 1.0f;
    cls.at<float>(0x510) = static_cast<float>(std::sin(static_cast<double>(cls.at<float>(0x1CC))));
    cls.at<float>(0x514) = static_cast<float>(std::sin(static_cast<double>(cls.at<float>(0x1D0))));
    cls.at<float>(0x518) = host.tangent_00412e20(cls.at<float>(0x1F0));
    cls.at<float>(0x51C) =
        host.tangent_00412e20(static_cast<float>(static_cast<double>(cls.at<float>(0x1F0)) * 0.699999988079071));
    const double a200 = cls.at<float>(0x200);
    const float s200 = static_cast<float>(std::sin(a200));
    const float c200 = static_cast<float>(std::cos(a200));
    cls.at<float>(0x528) = 0.0f;
    cls.at<float>(0x52C) = c200;
    cls.at<float>(0x530) = s200;
    const double a204 = cls.at<float>(0x204);
    const float s204 = static_cast<float>(std::sin(a204));
    const float c204 = static_cast<float>(std::cos(a204));
    cls.at<float>(0x53C) = s204;
    cls.at<float>(0x534) = 0.0f;
    cls.at<float>(0x538) = c204;
    cls.at<std::uint32_t>(0x4FC) = 0;
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    host.matrix_assign_004134f0(cls.sub(0x27C), identity);
    cls.at<float>(0x2AC) = 0.0f;
    cls.at<float>(0x2B0) = 1.2999999523162842f;
    cls.at<float>(0x2B4) = 3.0f;
    const float level = host.ai_tuning_0042e740().at<float>(0x24C) * cls.at<float>(0x184);
    cls.at<float>(0x1E4) = host.plane_class_solve_max_climb_angle_007d98f0(cls, level);
    cls.at<float>(0x1E8) = host.plane_class_solve_max_climb_angle_007d98f0(cls, cls.at<float>(0x18C));
    cls.at<float>(0x1EC) = static_cast<float>(static_cast<double>(cls.at<float>(0x1E4)) * 0.6000000238418579);
    const float radius = host.plane_class_leader_turn_radius_007db4d0(cls);
    cls.at<float>(0x26C) = radius;
    const float scaled_radius = static_cast<float>(static_cast<double>(radius) * 1.100000023841858);
    cls.at<float>(0x268) =
        host.max_float_by_ref_00415550(&host.ai_tuning_0042e740().at<float>(0x54C), &scaled_radius);
    const double factor = cls.at<float>(0x198) == 0.0f ? 2.200000047683716 : 1.5;
    const float cruise =
        static_cast<float>(static_cast<double>(host.plane_class_level_flight_speed_007c47f0(cls)) * factor);
    cls.at<float>(0x19C) = host.min_float_by_ref_00415510(&cls.at<float>(0x19C), &cruise);
    const float v1 = host.ai_tuning_0042e740().at<float>(0x24C) * cls.at<float>(0x184);
    const float c164 = cls.at<float>(0x164);
    const Raw bounds = cls.deref(0x50).sub(0x28);
    const float v2 = host.ai_tuning_0042e740().at<float>(0x24C) * cls.at<float>(0x184);
    cls.at<float>(0x1A0) =
        static_cast<float>(static_cast<double>(c164) / (static_cast<double>(v2) * v1 * 1.6));
    cls.at<float>(0x508) = -bounds.at<float>(4) - cls.at<float>(0x194);
    float negated = -0.0f - bounds.at<float>(0);
    cls.at<float>(0x500) = host.max_float_by_ref_00415550(&negated, &bounds.at<float>(0xC));
    negated = -0.0f - bounds.at<float>(4);
    float axis = host.max_float_by_ref_00415550(&negated, &bounds.at<float>(0x10));
    cls.at<float>(0x500) = host.max_float_by_ref_00415550(&cls.at<float>(0x500), &axis);
    negated = -0.0f - bounds.at<float>(8);
    axis = host.max_float_by_ref_00415550(&negated, &bounds.at<float>(0x14));
    cls.at<float>(0x500) = host.max_float_by_ref_00415550(&cls.at<float>(0x500), &axis);
    return host.vehicle_class_derive_constants_default_00951f20(cls);
}

// 007C2680, __thiscall(ECX = plane; peer), RET 4, SEH frame (vtable +17Ch).
// index = squadron +9D4h ? 007ED160(squadron; plane) : -1; flag = +AA0h > 0.0f
// (COMISS/JBE); +6B8h and the class are read, then vtable[34h] writes the
// world velocity straight into the by-value argument area; 007BD3E0(&create;
// class+70h, plane, +9D4h, index, +C54h, velocity, flag, +6B8h) (RET 28h);
// 00779FC0(plane; peer, &create). When any of +BCCh, +BD0h, +BD4h is not 1.0f
// (UCOMISS/JP, NaN counts as different): the seven dwords +BCCh..+BE4h are
// copied to a local, a 3Ch-byte C9h message is built (0075B430(C9h), +18h w0,
// +1Ah b0, +4 = 1, +1Ch b0, vptr 00D02E20, +20h..+38h = the local) and sent
// with 00779FC0(plane; peer, &msg), then its vptr is reset to 00CE4974. The
// create message's vptr is set to 00D0379C and 007668E0 destroys it.
void plane_send_create_message_007c2680(Seg47bHost& host, Raw plane, void* peer) {
    std::int32_t index = -1;
    if (plane.ptr(0x9D4) != nullptr) index = host.plane_squadron_member_index_of_007ed160(plane.deref(0x9D4), plane);
    const std::uint8_t flag = plane.at<float>(0xAA0) > 0.0f ? 1 : 0;
    const std::int32_t value_6b8 = plane.at<std::int32_t>(0x6B8);
    const Raw cls = plane.deref(0x538);
    float velocity[3];
    host.plane_vcall_world_velocity_34(plane, velocity);
    alignas(4) std::uint8_t create[0x12C];
    host.plane_create_message_construct_007bd3e0(Raw{create}, cls.at<std::uint32_t>(0x70), plane, plane.ptr(0x9D4),
                                                 index, plane.at<std::int32_t>(0xC54), velocity, flag, value_6b8);
    host.entity_send_message_to_peer_00779fc0(plane, peer, create);
    if (plane.at<float>(0xBCC) != 1.0f || plane.at<float>(0xBD0) != 1.0f || plane.at<float>(0xBD4) != 1.0f) {
        std::uint32_t local[7];
        std::memcpy(local, plane.field(0xBCC), sizeof local);
        alignas(4) std::uint8_t storage[0x3C];
        Raw message{storage};
        host.session_message_construct_0075b430(message, 0xC9);
        message.at<std::uint16_t>(0x18) = 0;
        message.at<std::uint8_t>(0x1A) = 0;
        message.at<std::uint32_t>(4) = 1;
        message.at<std::uint8_t>(0x1C) = 0;
        message.at<std::uint32_t>(0) = 0x00D02E20u;
        std::memcpy(message.field(0x20), local, sizeof local);
        host.entity_send_message_to_peer_00779fc0(plane, peer, storage);
        message.at<std::uint32_t>(0) = 0x00CE4974u;
    }
    Raw{create}.at<std::uint32_t>(0) = 0x00D0379Cu;
    host.net_entity_create_message_destruct_007668e0(create);
}

// 007C2880, __thiscall(ECX = plane; holder, force), RET 8, SEH frame.
// mine = +900h in 7/6/4/5 and +1B0h == ([holder] ? [[holder]+20h] : 8);
// ctl = 007D7A80(plane+AB0h) (its byte +94h); state = +900h; record =
// 007C2810(plane; holder). The send is skipped only when force == 0, mine,
// record+30h == 0, record+2Ch == 0, state == 7 and ctl == 0. Send: b904 =
// +904h, c8 = [[holder]+C8h], ctl2 = 007D7A80 again; the 3Ch block is filled
// in place by 007D8330(plane+AB0h; block); 007BDD30(&msg; plane,
// 007C2810(plane; holder) again, force, c8, plane+674h, plane+9E4h, block,
// +900h (re-read), mine, b904, ctl2); 00779FC0(plane; [holder], &msg); the
// message vptr is reset to 00CE4974. Always: 00779F80(plane; holder).
void plane_send_control_state_message_007c2880(Seg47bHost& host, Raw plane, void* holder, std::int32_t force) {
    const Raw h = raw(holder);
    std::uint8_t mine = 0;
    const std::int32_t current = plane.at<std::int32_t>(0x900);
    if (state_in_7645(current)) {
        const Raw peer = h.deref(0);
        const std::int32_t slot = peer ? peer.at<std::int32_t>(0x20) : 8;
        mine = plane.at<std::int32_t>(0x1B0) == slot ? 1 : 0;
    }
    const Raw controller = plane.sub(0xAB0);
    const std::uint8_t ctl = controller.at<std::uint8_t>(0x94);  // 007D7A80
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    const Raw record = raw(host.plane_state_record_get_or_create_007c2810(plane, holder));
    const bool skip = force == 0 && mine != 0 && record.at<std::int32_t>(0x30) == 0 &&
                      record.at<std::int32_t>(0x2C) == 0 && state == 7 && ctl == 0;
    if (!skip) {
        const std::uint8_t flag_904 = plane.at<std::uint8_t>(0x904);
        const std::uint8_t c8 = h.deref(0).at<std::uint8_t>(0xC8);
        const std::uint8_t ctl2 = controller.at<std::uint8_t>(0x94);  // 007D7A80
        const std::int32_t state_now = plane.at<std::int32_t>(0x900);
        alignas(4) std::uint8_t block[0x3C];
        host.flight_controller_write_state_record_007d8330(controller, block);
        void* const record2 = host.plane_state_record_get_or_create_007c2810(plane, holder);
        alignas(4) std::uint8_t storage[0xC0];
        void* const message = host.plane_sync_message_construct_007bdd30(
            Raw{storage}, plane, record2, force, c8, &plane.at<float>(0x674), &plane.at<float>(0x9E4), block,
            state_now, mine, flag_904, ctl2);
        host.entity_send_message_to_peer_00779fc0(plane, h.ptr(0), message);
        Raw{storage}.at<std::uint32_t>(0) = 0x00CE4974u;
    }
    host.net_entity_holder_stamp_sync_tick_00779f80(plane, holder);
}

// 007C29F0, __thiscall(ECX = plane; owner), RET 4, SEH frame (vtable +184h).
// b904 = +904h, ctl = 007D7A80(plane+AB0h); the 3Ch block from 007D8330;
// 007BDD30(&msg; plane, 007C2810(plane; owner), 0, 1, plane+674h,
// plane+9E4h, block, +900h, 0, b904, ctl); 00779F90(plane; msg). The image
// runs no destructor for the message after the send.
void plane_send_sync_message_007c29f0(Seg47bHost& host, Raw plane, void* owner) {
    const std::uint8_t flag_904 = plane.at<std::uint8_t>(0x904);
    const Raw controller = plane.sub(0xAB0);
    const std::uint8_t ctl = controller.at<std::uint8_t>(0x94);  // 007D7A80
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    alignas(4) std::uint8_t block[0x3C];
    host.flight_controller_write_state_record_007d8330(controller, block);
    void* const record = host.plane_state_record_get_or_create_007c2810(plane, owner);
    alignas(4) std::uint8_t storage[0xB4];
    void* const message = host.plane_sync_message_construct_007bdd30(
        Raw{storage}, plane, record, 0, 1, &plane.at<float>(0x674), &plane.at<float>(0x9E4), block, state, 0,
        flag_904, ctl);
    host.session_send_message_to_host_00779f90(plane, message);
}

// 007C3700, __thiscall(ECX = plane), RET 0.
// Head effect +AA4h (when set): kept and moved to the plane position +FCh
// (004842C0) while 00865FE0 says it is not finished and pose y +100h > -5.0f
// (00CFBC84, COMISS/JBE); otherwise released (InterlockedDecrement(+4) == 0 ->
// vtable[0]) and +AA4h cleared. Then with y < 14.0 (double 00D05AB0,
// FCOMI/JBE) every 10h-byte element {effect, offset[3]} of the vector
// +A3Ch (begin +A40h, end +A44h) is placed: P = 004142E0(offset, plane matrix
// +CCh), h = 0078CF20([[00E188A8]+19F0h]; P.x, P.z), d = (float)(P.y - h);
// when the effect is set it is 00866B70'd when 0.1 > d (double 00D7A3A0) and
// d > -4.0f (00CF1430), 00867B10'd otherwise, moved to (P.x, (float)(h +
// 0.1), P.z) and its +54h set to 007D99C0(plane+AB0h). With y >= 14 (or NaN)
// and 20.0f (00CE3930) > y every set effect is 00867B10'd. The checked-iterator
// tests (_invalid_parameter_noinfo) are kept at the image's points.
void plane_update_bow_wave_effect_007c3700(Seg47bHost& host, Raw plane) {
    if (plane.ptr(0xAA4) != nullptr) {
        bool release = host.point_effect_is_finished_00865fe0(plane.deref(0xAA4));
        if (!release) {
            refresh_pose(host, plane);
            if (plane.at<float>(0x100) > -5.0f) {
                refresh_pose(host, plane);
                host.point_effect_set_world_point_004842c0(plane.deref(0xAA4), &plane.at<float>(0xFC));
            } else {
                release = true;
            }
        }
        if (release) {
            const Raw head = plane.deref(0xAA4);
            if (host.interlocked_decrement_ce2220(&head.at<long>(4)) == 0) host.refcounted_vcall_destroy_00(head);
            plane.ptr(0xAA4) = nullptr;
        }
    }
    refresh_pose(host, plane);
    const auto begin_end_check = [&]() -> std::uint8_t* {
        std::uint8_t* const end = static_cast<std::uint8_t*>(plane.ptr(0xA44));
        if (static_cast<std::uint8_t*>(plane.ptr(0xA40)) > end) host.invalid_parameter_00bf6713();
        return end;
    };
    if (14.0 > static_cast<double>(plane.at<float>(0x100))) {
        std::uint8_t* it = static_cast<std::uint8_t*>(plane.ptr(0xA40));
        if (it > static_cast<std::uint8_t*>(plane.ptr(0xA44))) host.invalid_parameter_00bf6713();
        for (;;) {
            std::uint8_t* const end = begin_end_check();
            if (it == end) break;
            refresh_pose(host, plane);
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            float p[3];
            host.vector3_transform_affine_point_004142e0(reinterpret_cast<const float*>(it + 4), p,
                                                         plane.field(0xCC));
            const Raw game = raw(host.global_pointer(0x00E188A8u));
            const float h = host.game_world_sample_water_height_0078cf20(game.deref(0x19F0), p[0], p[2]);
            const float d = p[1] - h;
            const bool near_surface = 0.10000000149011612 > static_cast<double>(d) && d > -4.0f;
            const float q[3] = {p[0], static_cast<float>(static_cast<double>(h) + 0.10000000149011612), p[2]};
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            const Raw element{it};
            if (element.ptr(0) != nullptr) {
                check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
                if (near_surface) {
                    host.effect_handle_request_stop_00866b70(element.deref(0));
                } else {
                    host.point_effect_stop_children_00867b10(element.deref(0));
                }
                check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
                host.point_effect_set_world_point_004842c0(element.deref(0), q);
                check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
                const Raw effect = element.deref(0);
                effect.at<float>(0x54) = host.plane_flight_forward_speed_007d99c0(plane.sub(0xAB0));
            }
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            it += 0x10;
        }
        return;
    }
    refresh_pose(host, plane);
    if (!(20.0f > plane.at<float>(0x100))) return;
    std::uint8_t* it = static_cast<std::uint8_t*>(plane.ptr(0xA40));
    if (it > static_cast<std::uint8_t*>(plane.ptr(0xA44))) host.invalid_parameter_00bf6713();
    for (;;) {
        std::uint8_t* const end = begin_end_check();
        if (it == end) break;
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
        const Raw element{it};
        if (element.ptr(0) != nullptr) {
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            host.point_effect_stop_children_00867b10(element.deref(0));
        }
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
        it += 0x10;
    }
}

// 007C39B0, (ECX = plane, EDX = target), RET 0; no caller in the image.
// s = 007C2610(plane); A = plane position +FCh, B = target position +FCh
// (each after the pose refresh); t1 = (float)(|B - A| / s); P1 =
// target->vtable[48h](t1); t2 = (float)(|P1 - A| / s); P2 = vtable[48h](t2).
// The red colour {1,0,0,1} at 00F872AC is initialised once under guard bit 1
// of 00F872BC (the image inlines the guarded store twice; the second copy
// never runs). Then 0085F990(B, P2, red, red) and 00860BE0(P2, red; 3.0f
// (00CE3854), red, 0). The quotients are x87 divides rounded to float.
void plane_debug_draw_gun_lead_007c39b0(Seg47bHost& host, Raw plane, Raw target) {
    const float s = host.unit_min_kind21_component_speed_007c2610(plane);
    refresh_pose(host, plane);
    const float a[3] = {plane.at<float>(0xFC), plane.at<float>(0x100), plane.at<float>(0x104)};
    refresh_pose(host, target);
    const float b[3] = {target.at<float>(0xFC), target.at<float>(0x100), target.at<float>(0x104)};
    const float t1 = length_or_zero(b[0] - a[0], b[1] - a[1], b[2] - a[2]) / s;
    float p1[3];
    host.unit_predict_position_vcall_48(target, p1, t1);
    const float t2 = length_or_zero(p1[0] - a[0], p1[1] - a[1], p1[2] - a[2]) / s;
    float p2[3];
    host.unit_predict_position_vcall_48(target, p2, t2);
    auto& guard = *static_cast<std::uint32_t*>(host.global_address(0x00F872BCu));
    float* const red = static_cast<float*>(host.global_address(0x00F872ACu));
    const auto init_red = [&] {
        if ((guard & 1u) == 0) {
            guard |= 1u;
            red[0] = 1.0f;
            red[1] = 0.0f;
            red[2] = 0.0f;
            red[3] = 1.0f;
        }
    };
    init_red();
    float c0[4];
    float c1[4];
    std::memcpy(c0, red, sizeof c0);
    std::memcpy(c1, red, sizeof c1);
    host.debug_draw_line_rgba_0085f990(b, p2, c0, c1);
    init_red();
    host.debug_draw_wire_sphere_00860be0(p2, red, 3.0f, red, 0);
}

// 007C6E10, __thiscall(ECX = plane), RET 0. 0 unless (plane+72Ch)->vtable[38h]()
// answers; r = (float)(plane->vtable[38h]() / class(+538h)+184h); 2 when
// tuning(0042E740)+22Ch > r, else 1 when tuning+230h > r (both FCOMI/JBE, so
// a NaN r gives 0).
std::int32_t plane_stall_warning_level_007c6e10(Seg47bHost& host, Raw plane) {
    if (!host.control_mode_vcall_38(plane.sub(0x72C))) return 0;
    const Raw cls = plane.deref(0x538);
    const float r = host.plane_vcall_speed_38(plane) / cls.at<float>(0x184);
    if (host.ai_tuning_0042e740().at<float>(0x22C) > r) return 2;
    if (host.ai_tuning_0042e740().at<float>(0x230) > r) return 1;
    return 0;
}

// 007C74A0, __thiscall(ECX = plane), RET 0, SEH frame (the lock guard).
// cs = [007C2EC0()+4]; when set EnterCriticalSection(cs) ([00CE2218]) and
// cs+18h += 1. plane+C40h = 1. Append the plane to the global pointer vector
// {data [00F87278], size [00F8727C], capacity [00F87280]}: when size ==
// capacity, capacity = 2 * capacity + 2, a new block 00BF55BE(capacity * 4)
// receives the elements (each store skipped for a null destination, the
// image's placement-new test), the old block is freed (00BF6989, when set)
// and replaces data. The new slot is written unless data + size*4 is null;
// size += 1. Then cs+18h -= 1 and LeaveCriticalSection(cs) ([00CE2210]).
void plane_register_firing_guns_007c74a0(Seg47bHost& host, Raw plane) {
    void* const cs = raw(cc13_seg47::firing_gun_registry_instance_007c2ec0(host)).ptr(4);
    if (cs != nullptr) {
        host.enter_critical_section_ce2218(cs);
        raw(cs).at<std::int32_t>(0x18) += 1;
    }
    plane.at<std::uint8_t>(0xC40) = 1;
    auto& data = *static_cast<std::uint32_t**>(host.global_address(0x00F87278u));
    auto& size = *static_cast<std::uint32_t*>(host.global_address(0x00F8727Cu));
    auto& capacity = *static_cast<std::uint32_t*>(host.global_address(0x00F87280u));
    if (size == capacity) {
        capacity = capacity + capacity + 2;
        auto* const fresh = static_cast<std::uint32_t*>(host.operator_new_00bf55be(capacity * 4));
        std::uint32_t* dst = fresh;
        for (std::uint32_t i = 0; i < size; ++i, ++dst) {
            if (dst != nullptr) *dst = data[i];
        }
        if (data != nullptr) host.free_00bf6989(data);
        data = fresh;
    }
    std::uint32_t* const slot = data + size;
    if (slot != nullptr) *slot = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(plane.p));
    size += 1;
    if (cs != nullptr) {
        raw(cs).at<std::int32_t>(0x18) -= 1;
        host.leave_critical_section_ce2210(cs);
    }
}

// 007C78A0, __thiscall(ECX = plane; state, flag byte), RET 8: the client-side
// flight-state set (no message). +904h = flag; nothing more when +900h ==
// state. Else +C04h = -1.0f (00D7A260), +900h = state, and with "ground" =
// state in 1..5:
// - air -> ground: 00954580(plane); 007C5AC0(plane; -1.0f) when +BF4h is null;
//   parent = [[+BF4h]+4] ? [[+BF4h]+4]+7Ch : 0 (the image dereferences +BF4h
//   without a test); when set: 007D9CE0(+AB0h; parent), 007BA020(+74h;
//   00414E10(parent)), 0042ED50(plane), (+310h)->vtable[0Ch](),
//   plane->vtable[ACh](parent), 0042ED50(plane), (+310h)->vtable[0Ch](),
//   007D9C10(+AB0h).
// - ground -> air, when 00923810(plane; 1) is non-null: the same sequence with
//   parent 0 and 004134F0(+74h; 0042D7E0(plane)) in place of 007BA020.
void plane_set_flight_state_unguarded_007c78a0(Seg47bHost& host, Raw plane, std::int32_t state, std::uint8_t flag) {
    plane.at<std::uint8_t>(0x904) = flag;
    const std::int32_t old = plane.at<std::int32_t>(0x900);
    if (old == state) return;
    plane.at<float>(0xC04) = -1.0f;
    const bool was_ground = old >= 1 && old <= 5;
    plane.at<std::int32_t>(0x900) = state;
    const bool now_ground = state >= 1 && state <= 5;
    const Raw controller = plane.sub(0xAB0);
    const Raw body = plane.sub(0x310);
    if (!was_ground) {
        if (!now_ground) return;
        host.unit_00954580(plane);
        if (plane.ptr(0xBF4) == nullptr) host.plane_landing_site_probe_007c5ac0(plane, -1.0f);
        const Raw holder = plane.deref(0xBF4).deref(4);
        void* const parent = holder ? holder.ptr(0x7C) : nullptr;
        if (parent == nullptr) return;
        host.flight_controller_reparent_velocity_007d9ce0(controller, parent);
        host.matrix_multiply_in_place_007ba020(plane.sub(0x74),
                                               host.entity_pose_derived_affine_inverse_00414e10(raw(parent)));
        host.scene_node_invalidate_subtree_pose_0042ed50(plane);
        host.plane_body_vcall_0c(body);
        host.plane_vcall_set_parent_ac(plane, parent);
        host.scene_node_invalidate_subtree_pose_0042ed50(plane);
        host.plane_body_vcall_0c(body);
        host.flight_controller_refresh_body_frame_007d9c10(controller);
        return;
    }
    if (now_ground) return;
    if (host.entity_ancestor_00923810(plane, 1) == nullptr) return;
    host.flight_controller_reparent_velocity_007d9ce0(controller, nullptr);
    host.matrix_assign_004134f0(plane.sub(0x74), host.entity_pose_world_matrix_0042d7e0(plane));
    host.scene_node_invalidate_subtree_pose_0042ed50(plane);
    host.plane_body_vcall_0c(body);
    host.plane_vcall_set_parent_ac(plane, nullptr);
    host.scene_node_invalidate_subtree_pose_0042ed50(plane);
    host.plane_body_vcall_0c(body);
    host.flight_controller_refresh_body_frame_007d9c10(controller);
}

// 007C95A0, __thiscall(ECX = plane), RET 0, SEH frame: the nose-art decal.
// model = [node+0Ch] with node = [plane+360h] ? [[plane+360h]+160h] : 0 (read
// without a null test). A local NativeString S = {0, 0}. Session mode
// [[00E188A8]+1FE4h] != 0: when slot +180h <= 7 (signed), tmp =
// 007F9F70([game+slot*4+18CCh]; out, [that+28h]); unless &S == tmp,
// 0041DD40(&S; tmp.len, 1) and, when tmp.len != 0, memcpy(S.data, tmp.data,
// S.len); tmp is released; with slot > 7 nothing but S's release. Mode 0:
// 0041DD40(&S; 0, 0) and, when S.data is set, memcpy(S.data, "" (00CE3A0C),
// S.len). When S.len != 0: t = [00F8D394]->vtable[64h](&S, 0); name =
// 0041E870("pimpdecal.shfx" 00D05B70); 00B85310(model, &name; t, 0); name
// released; t released (InterlockedDecrement(t+4) == 0 -> vtable[0]) when
// set. S released last.
void plane_apply_nose_art_decal_007c95a0(Seg47bHost& host, Raw plane) {
    const Raw holder = plane.deref(0x360);
    const Raw node = holder ? holder.deref(0x160) : Raw{nullptr};
    const Raw model = node.deref(0xC);
    alignas(4) std::uint8_t s_storage[8] = {};
    const Raw s{s_storage};
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    if (game.at<std::int32_t>(0x1FE4) != 0) {
        const std::int32_t slot = plane.at<std::int32_t>(0x180);
        if (slot > 7) {
            native_string_release(host, s);
            return;
        }
        const Raw table = game.deref(0x18CC + slot * 4);
        alignas(4) std::uint8_t tmp_storage[8];
        const Raw tmp = host.nose_art_texture_path_007f9f70(table, Raw{tmp_storage}, table.at<std::int32_t>(0x28));
        if (s.p != tmp.p) {
            host.native_string_resize_0041dd40(s, tmp.at<std::uint32_t>(0), 1);
            if (tmp.at<std::uint32_t>(0) != 0) {
                host.memcpy_00bf7680(s.ptr(4), tmp.ptr(4), s.at<std::uint32_t>(0));
            }
        }
        native_string_release(host, Raw{tmp_storage});
    } else {
        host.native_string_resize_0041dd40(s, 0, 0);
        if (s.ptr(4) != nullptr) host.memcpy_00bf7680(s.ptr(4), "", s.at<std::uint32_t>(0));
    }
    if (s.at<std::uint32_t>(0) != 0) {
        const Raw renderer = raw(host.global_pointer(0x00F8D394u));
        void* const texture = host.renderer_vcall_load_texture_64(renderer, s, 0);
        alignas(4) std::uint8_t name_storage[8];
        const Raw name{name_storage};
        host.native_string_assign_0041e870(name, "pimpdecal.shfx");
        host.model_set_texture_slot_by_effect_00b85310(model, name, texture, 0);
        native_string_release(host, name);
        if (texture != nullptr) {
            const Raw t = raw(texture);
            if (host.interlocked_decrement_ce2220(&t.at<long>(4)) == 0) host.refcounted_vcall_destroy_00(t);
        }
    }
    native_string_release(host, s);
}

// 007C9770, __thiscall(ECX = plane), RET 0, SEH frame: the plane's SEntity
// init pass C. 0095E5B0, 00951F80, 007C95A0, 007BC550 (each on the plane);
// 00922F80(plane; 0) when +900h is 1 or 0; when the squadron +9D4h is set,
// key = 0041E870("SquadronID" 00D05B80), 00B67460(00927B40(plane; &lua);
// &key, (uint16)[+9D4h]+174h), 00B67700(&lua), key released; finally
// 007C5AC0(plane; -1.0f).
void plane_sentity_init_pass_c_007c9770(Seg47bHost& host, Raw plane) {
    host.unit_read_common_lua_properties_0095e5b0(plane);
    host.unit_00951f80(plane);
    plane_apply_nose_art_decal_007c95a0(host, plane);
    host.plane_sync_elevator_attachment_007bc550(plane);
    const std::int32_t state = plane.at<std::int32_t>(0x900);
    if (state == 1 || state == 0) host.scene_node_disable_00922f80(plane, 0);
    if (plane.ptr(0x9D4) != nullptr) {
        alignas(4) std::uint8_t key_storage[8];
        const Raw key{key_storage};
        host.native_string_assign_0041e870(key, "SquadronID");
        alignas(4) std::uint8_t lua[0x10];
        const Raw object = host.mission_entity_lua_self_object_00927b40(plane, lua);
        host.lua_object_set_string_integer_00b67460(object, key, plane.deref(0x9D4).at<std::uint16_t>(0x174));
        host.lua_object_destruct_00b67700(lua);
        native_string_release(host, key);
    }
    host.plane_landing_site_probe_007c5ac0(plane, -1.0f);
}

// 007CA5F0, __thiscall(ECX = plane; bool firing), RET 4, SEH frame.
// 00956EE0(plane; firing) first. Firing with the effect vector {data +A30h,
// count +A34h} empty: for i < class(+538h)+23Ch (re-read each pass), one
// point effect per gun: definition class+234h passed by value (its count +4
// InterlockedIncrement'ed when set; 008687C0 releases it), the gun's 40h-byte
// matrix class+238h + 40h*i, node +4A4h, words (1, 0, 0); the new reference is
// appended (0074D780(+A30h; &ref)) and the local reference released
// (InterlockedDecrement(+4) == 0 -> vtable[0]). Not firing: while the count is
// non-zero, the last effect is 00867B10'd and its byte +9 set to 1, then
// popped (released and its slot zeroed when non-null; count -= 1).
void plane_set_gun_fire_effects_007ca5f0(Seg47bHost& host, Raw plane, bool firing) {
    host.plane_set_gun_fire_effect_00956ee0(plane, firing);
    if (firing) {
        if (plane.at<std::int32_t>(0xA34) != 0) return;
        for (std::int32_t i = 0; i < plane.deref(0x538).at<std::int32_t>(0x23C); ++i) {
            const Raw cls = plane.deref(0x538);
            const Raw node = plane.deref(0x4A4);
            void* const definition = cls.ptr(0x234);
            if (definition != nullptr) host.interlocked_increment_ce221c(&raw(definition).at<long>(4));
            alignas(4) std::uint8_t ref_storage[4] = {};
            void* const ref = host.point_effect_create_with_parent_matrix_008687c0(
                ref_storage, node, definition, cls.field(0x238 + 0x40 * i), 1, 0, 0);
            host.live_effect_references_append_0074d780(plane.sub(0xA30), ref);
            void* const created = Raw{ref_storage}.ptr(0);
            if (created != nullptr) {
                if (host.interlocked_decrement_ce2220(&raw(created).at<long>(4)) == 0) {
                    host.refcounted_vcall_destroy_00(raw(created));
                }
            }
        }
        return;
    }
    stop_and_pop_gun_effects(host, plane);
}

// 007CC580, __thiscall(ECX = plane), RET 0 (vtable[84h], the kill): ends in a
// tail jump to the base 0095D400(plane).
// plane->vtable[10h]() (the result is not used); 007C75A0(plane); every set
// effect of the 10h-byte vector +A3Ch (begin +A40h, end +A44h) is 00867B10'd,
// its byte +9 set, released (InterlockedDecrement(+4) == 0 -> vtable[0]) and
// its slot zeroed (the image stores the zero twice); every set effect of the
// vector +A4Ch (begin +A50h, end +A54h) is 00867B10'd with byte +9 set (not
// released); for i < class(+538h)+5A0h (re-read each pass) the effect at
// +A5Ch + 10h*i likewise; then the +A30h gun effects are stopped and popped
// as in 007CA5F0. The checked-iterator tests are kept; the container-identity
// tests compare a vector with itself and cannot fail.
void plane_on_killed_007cc580(Seg47bHost& host, Raw plane) {
    host.entity_vcall_name_10(plane);
    plane_unregister_firing_guns_007c75a0(host, plane);
    std::uint8_t* it = static_cast<std::uint8_t*>(plane.ptr(0xA40));
    if (it > static_cast<std::uint8_t*>(plane.ptr(0xA44))) host.invalid_parameter_00bf6713();
    for (;;) {
        std::uint8_t* const end = static_cast<std::uint8_t*>(plane.ptr(0xA44));
        if (static_cast<std::uint8_t*>(plane.ptr(0xA40)) > end) host.invalid_parameter_00bf6713();
        if (it == end) break;
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
        const Raw element{it};
        if (element.ptr(0) != nullptr) {
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            const Raw effect = element.deref(0);
            host.point_effect_stop_children_00867b10(effect);
            effect.at<std::uint8_t>(9) = 1;
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
            if (element.ptr(0) != nullptr) {
                const Raw p = element.deref(0);
                if (host.interlocked_decrement_ce2220(&p.at<long>(4)) == 0) host.refcounted_vcall_destroy_00(p);
                element.ptr(0) = nullptr;
            }
            element.ptr(0) = nullptr;
        }
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA44)));
        it += 0x10;
    }
    it = static_cast<std::uint8_t*>(plane.ptr(0xA50));
    if (it > static_cast<std::uint8_t*>(plane.ptr(0xA54))) host.invalid_parameter_00bf6713();
    for (;;) {
        std::uint8_t* const end = static_cast<std::uint8_t*>(plane.ptr(0xA54));
        if (static_cast<std::uint8_t*>(plane.ptr(0xA50)) > end) host.invalid_parameter_00bf6713();
        if (it == end) break;
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA54)));
        const Raw element{it};
        if (element.ptr(0) != nullptr) {
            check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA54)));
            const Raw effect = element.deref(0);
            host.point_effect_stop_children_00867b10(effect);
            effect.at<std::uint8_t>(9) = 1;
        }
        check_before_end(host, it, static_cast<std::uint8_t*>(plane.ptr(0xA54)));
        it += 0x10;
    }
    for (std::int32_t i = 0; i < plane.deref(0x538).at<std::int32_t>(0x5A0); ++i) {
        void* const p = plane.ptr(0xA5C + 0x10 * i);
        if (p != nullptr) {
            host.point_effect_stop_children_00867b10(raw(p));
            raw(p).at<std::uint8_t>(9) = 1;
        }
    }
    stop_and_pop_gun_effects(host, plane);
    host.unit_on_killed_0095d400(plane);
}

// 007CB440, __thiscall(ECX = plane; out), RET 4, EAX = out, SEH frame; no
// direct caller (a vtable slot). With a squadron +9D4h: out = plane name +
// " of " (00D05E20) + squadron name, built as A = sq->vtable[10h](), B = " of ",
// C = plane->vtable[10h](), T = 004261A0(C; T, B), 004261A0(T; out, A); then T,
// C, B, A are released in that order. Without: A = " zombie" (00D05E18), B =
// plane name, out = 004261A0(B; out, A); B then A released.
Raw plane_format_debug_name_007cb440(Seg47bHost& host, Raw plane, Raw out) {
    alignas(4) std::uint8_t a_storage[8];
    alignas(4) std::uint8_t b_storage[8];
    const Raw a{a_storage};
    const Raw b{b_storage};
    if (plane.ptr(0x9D4) != nullptr) {
        host.native_string_assign_0041e870(a, host.entity_vcall_name_10(plane.deref(0x9D4)));
        host.native_string_assign_0041e870(b, " of ");
        alignas(4) std::uint8_t c_storage[8];
        alignas(4) std::uint8_t t_storage[8];
        const Raw c{c_storage};
        const Raw t{t_storage};
        host.native_string_assign_0041e870(c, host.entity_vcall_name_10(plane));
        const Raw joined = host.native_string_concat_004261a0(c, t, b);
        host.native_string_concat_004261a0(joined, out, a);
        native_string_release(host, t);
        native_string_release(host, c);
        native_string_release(host, b);
    } else {
        host.native_string_assign_0041e870(a, " zombie");
        host.native_string_assign_0041e870(b, host.entity_vcall_name_10(plane));
        host.native_string_concat_004261a0(b, out, a);
        native_string_release(host, b);
    }
    native_string_release(host, a);
    return out;
}

// 007CCA50, no register input, RET 0, SEH frame. The pending list is the
// vector object at 00F87284 (begin [00F87288], end [00F8728C]); nothing when
// begin is null or the list is empty. Otherwise a 220h-byte stack message:
// 0075B430(C2h), +4 = 0, vptr 00D034F0, +18h = 0, sent with
// 00770AF0([00E188A8]+1EF0h; &message) (its serializer is the vptr's; no
// destructor runs after); then the list is cleared with 00489370(00F87284;
// &out, {00F87284, begin}, {00F87284, end}), each bound re-read and the
// begin > end checked-iterator test kept twice.
void plane_send_sync_ack_batch_007cca50(Seg47bHost& host) {
    auto* const begin_slot = static_cast<std::uint8_t**>(host.global_address(0x00F87288u));
    auto* const end_slot = static_cast<std::uint8_t**>(host.global_address(0x00F8728Cu));
    if (*begin_slot == nullptr) return;
    if (((*end_slot - *begin_slot) >> 2) == 0) return;
    alignas(4) std::uint8_t storage[0x220];
    Raw message{storage};
    host.session_message_construct_0075b430(message, 0xC2);
    message.at<std::uint32_t>(4) = 0;
    message.at<std::uint32_t>(0) = 0x00D034F0u;
    message.at<std::uint32_t>(0x18) = 0;
    const Raw game = raw(host.global_pointer(0x00E188A8u));
    host.session_send_to_secondary_first_peer_00770af0(game.sub(0x1EF0), storage);
    std::uint8_t* const last = *end_slot;
    if (*begin_slot > *end_slot) host.invalid_parameter_00bf6713();
    std::uint8_t* const first = *begin_slot;
    if (first > *end_slot) host.invalid_parameter_00bf6713();
    void* const list = host.global_address(0x00F87284u);
    alignas(4) std::uint8_t out[8];
    host.ptr_vector_erase_range_00489370(list, out, list, first, list, last);
}

// 007CCFA0, __thiscall(ECX = plane; message), RET 4, AL (the plane's message
// handler). message->vtable[0Ch](62h) -> plane->vtable[208h](message), 1.
// Otherwise on the kind byte +10h (jump tables 007CD1D8 / 007CD1AC):
//   7Ah 007B83F0(byte +20h); C4h 007BBBA0; C5h 007C71E0; C6h 007C6F50(byte
//   +20h); C7h 007CC8B0(007BA890(message)); C8h 007BA600(plane+AB0h;
//   {+20h, +24h, +28h}) (cc13_seg47 names that parameter `plane`; the image
//   passes the controller); C9h the seven dwords +20h..+38h -> plane+BCCh;
//   CAh 007BBA10(message); CCh 007BB170(dword +24h, float +20h); any other
//   kind -> 0095ABE0(plane; message), whose AL is returned.
//   C3h, on the requested state byte +21h (table 007CD22C; current = +20h):
//   1 -> 007CC820; 2 -> 007CC7A0; 4 / 5 -> 007CB5F0 when current is 7, else
//   007C7430 when 0042A7E0 answers, else 007CA3F0(0 for 4, 1 for 5); 6 ->
//   007CB9E0; 7 -> 007C7110; 3 and anything else -> nothing.
// Every handled kind returns 1.
bool plane_handle_message_007ccfa0(Seg47bHost& host, Raw plane, Raw message) {
    if (host.message_vcall_is_kind_0c(message, 0x62)) {
        host.plane_vcall_handle_kind62_208(plane, message);
        return true;
    }
    switch (message.at<std::uint8_t>(0x10)) {
    case 0x7A:
        host.plane_set_launch_delay_aa0_007b83f0(plane, message.at<std::uint8_t>(0x20));
        return true;
    case 0xC3: {
        const std::uint8_t current = message.at<std::uint8_t>(0x20);
        switch (message.at<std::uint8_t>(0x21)) {
        case 1:
            host.plane_enter_flight_state_one_007cc820(plane);
            break;
        case 2:
            host.plane_enter_flight_state_two_007cc7a0(plane);
            break;
        case 4:
        case 5:
            if (current == 7) {
                host.plane_on_touchdown_from_flight_007cb5f0(plane);
            } else if (host.plane_is_on_ground_0042a7e0(plane)) {
                cc13_seg47::plane_remote_touchdown_to_state4_007c7430(host, plane);
            } else {
                host.plane_handle_touchdown_or_crash_007ca3f0(plane, message.at<std::uint8_t>(0x21) == 5 ? 1 : 0);
            }
            break;
        case 6:
            cc13_seg47::plane_remote_enter_flight_state6_007cb9e0(host, plane);
            break;
        case 7:
            host.plane_begin_flying_007c7110(plane);
            break;
        default:
            break;
        }
        return true;
    }
    case 0xC4:
        host.unit_request_ordnance_release_007bbba0(plane);
        return true;
    case 0xC5:
        host.plane_touchdown_007c71e0(plane);
        return true;
    case 0xC6:
        host.plane_detach_from_parent_keep_world_007c6f50(plane, message.at<std::uint8_t>(0x20));
        return true;
    case 0xC7:
        host.plane_return_to_stock_on_disappear_007cc8b0(
            plane, cc13_seg47::session_message_resolve_entity_by_net_id_007ba890(host, message));
        return true;
    case 0xC8: {
        const float v[3] = {message.at<float>(0x20), message.at<float>(0x24), message.at<float>(0x28)};
        cc13_seg47::plane_store_message_vector_f0_007ba600(plane.sub(0xAB0), v);
        return true;
    }
    case 0xC9:
        std::memcpy(plane.field(0xBCC), message.field(0x20), 7 * 4);
        return true;
    case 0xCA:
        cc13_seg47::plane_on_impact_message_007bba10(host, plane, message);
        return true;
    case 0xCC:
        cc13_seg47::plane_set_pilot_command_last_time_007bb170(host, plane, message.at<std::int32_t>(0x24),
                                                               message.at<float>(0x20));
        return true;
    default:
        return host.unit_handle_message_0095abe0(plane, message);
    }
}

// 007CD930, __thiscall(ECX = plane), RET 0. +C24h (PilotFires) = 0; for each
// child in the list [plane+48h] -> [child+44h]: when child->vtable[5Ch](20h)
// answers, the platform index i = child+38Ch indexes the class vector
// class(+538h)+94h {data +0, count +4}; when i >= count (signed JL) the vector
// is grown with 005471B0(vector; i + 1) first; a non-zero byte +0Ch of
// data[i] sets +C24h = 1 (the walk continues to the end).
void plane_compute_pilot_fires_007cd930(Seg47bHost& host, Raw plane) {
    plane.at<std::uint8_t>(0xC24) = 0;
    for (Raw child = plane.deref(0x48); child; child = child.deref(0x44)) {
        if (!host.entity_vcall_is_kind_5c(child, 0x20)) continue;
        const Raw vector = plane.deref(0x538).sub(0x94);
        const std::int32_t index = child.at<std::int32_t>(0x38C);
        if (!(index < vector.at<std::int32_t>(4))) host.class_vector_grow_005471b0(vector, index + 1);
        const Raw platform = raw(static_cast<void**>(vector.ptr(0))[index]);
        if (platform.at<std::uint8_t>(0xC) != 0) plane.at<std::uint8_t>(0xC24) = 1;
    }
}

// 007CDF20, __thiscall(ECX = plane), RET 0. 0077E830(plane) first; then on the
// creator c = [plane+C0h] kind [c+4]:
// - 1 (a scene bag b = [c+8]): when 0048E9F0(b; "Equipment" 00CF69AC):
//   type = 008F2260(b; "Type" 00CE4780)+0Ch, e = 008F2260(b; "Equipment")+0Ch,
//   cls = 00964790(type, DL = 1) (the image passes the whole dword in ECX),
//   +C54h = e, then 0095A880(plane; e > 0 (signed) ? 009552E0(cls; e) : 0, 1,
//   0); without the key 0095A880(plane; 0, 1, 0) and +C54h is not written.
// - 2 (a record d = [c+8]): +C54h = d+124h; when the word d+110h is non-zero
//   and 00521E30 resolves it: +9D4h = squadron, +9D8h = d+120h,
//   007ED0D0(squadron; plane, d+120h).
// - otherwise nothing.
// Then 007CDC70(plane), and every child of the list [plane+48h] -> [+44h] whose
// vtable[5Ch](25h) answers is appended to the array +974h (count +994h, no
// bound test in the image).
void plane_attach_lua_and_read_type_007cdf20(Seg47bHost& host, Raw plane) {
    host.mission_entity_attach_lua_self_0077e830(plane);
    const Raw creator = plane.deref(0xC0);
    const std::int32_t kind = creator.at<std::int32_t>(4);
    if (kind == 1) {
        void* const bag = creator.ptr(8);
        if (host.scene_property_bag_contains_0048e9f0(bag, "Equipment")) {
            const std::int32_t type = host.property_bag_find_008f2260(plane.deref(0xC0).ptr(8), "Type")
                                          .at<std::int32_t>(0xC);
            const std::int32_t equipment =
                host.property_bag_find_008f2260(plane.deref(0xC0).ptr(8), "Equipment").at<std::int32_t>(0xC);
            const Raw cls = raw(host.entity_class_by_id_00964790(static_cast<std::uint16_t>(type), 1));
            plane.at<std::int32_t>(0xC54) = equipment;
            void* devices = nullptr;
            if (equipment > 0) devices = host.plane_class_device_set_009552e0(cls, equipment);
            host.unit_instantiate_class_devices_0095a880(plane, devices, 1, 0);
        } else {
            host.unit_instantiate_class_devices_0095a880(plane, nullptr, 1, 0);
        }
    } else if (kind == 2) {
        const Raw record = creator.deref(8);
        plane.at<std::int32_t>(0xC54) = record.at<std::int32_t>(0x124);
        const std::uint16_t id = record.at<std::uint16_t>(0x110);
        if (id != 0) {
            void* const squadron = host.entity_handle_table_resolve_00521e30(id);
            if (squadron != nullptr) {
                plane.ptr(0x9D4) = squadron;
                const std::int32_t order = record.at<std::int32_t>(0x120);
                plane.at<std::int32_t>(0x9D8) = order;
                host.plane_squadron_insert_plane_sorted_007ed0d0(raw(squadron), plane, order);
            }
        }
    }
    host.plane_publish_step_record_007cdc70(plane);
    for (Raw child = plane.deref(0x48); child; child = child.deref(0x44)) {
        if (!host.entity_vcall_is_kind_5c(child, 0x25)) continue;
        const std::int32_t count = plane.at<std::int32_t>(0x994);
        plane.ptr(0x974 + count * 4) = child.p;
        plane.at<std::int32_t>(0x994) = count + 1;
    }
}

// 007CFA30, __thiscall(ECX = plane class), RET 0, SEH frame: the plane
// vehicle-class destructor (vptrs 00D05EAC at +0 and 00D05EA8 at +6Ch), each
// member in the image's order:
//   +5BCh pod vector: 00BF65AC(data) when set, three words zeroed;
//   +5A4h reference released (InterlockedDecrement(+4) == 0 -> vtable[0]), zeroed;
//   vectors +560h and +550h {first +4, last +8, end +0Ch}: when first is set,
//     007CD3F0(first, last; vector, allocator) and 00BF65AC(first); words zeroed;
//   007CCD30(+540h);
//   +250h: 007C9290(+250h; 0), then 00BF6989([+250h]);
//   +238h {data, size, capacity}: 007BD600(+238h; 0) when capacity < 0; the
//     size is counted down to 0 one by one and stored 0; 00BF6989(data);
//   +234h reference released and zeroed;
//   vector +224h: 004CC760(first, last; vector, allocator), 00BF65AC(first), zeroed;
//   references +220h, +218h, +214h, +210h released and zeroed;
//   +14Ch pod vector deleted and zeroed; NativeString +138h released;
//   finally 0095F1C0(class).
// The allocator byte the image pushes is an uninitialised stack byte read by
// nothing; 0 here.
void plane_class_base_destruct_007cfa30(Seg47bHost& host, Raw cls) {
    cls.at<std::uint32_t>(0) = 0x00D05EACu;
    cls.at<std::uint32_t>(0x6C) = 0x00D05EA8u;
    const auto release_ref = [&](std::size_t off) {
        void* const p = cls.ptr(off);
        if (p == nullptr) return;
        if (host.interlocked_decrement_ce2220(&raw(p).at<long>(4)) == 0) host.refcounted_vcall_destroy_00(raw(p));
        cls.ptr(off) = nullptr;
    };
    const auto zero3 = [&](std::size_t off) {
        cls.at<std::uint32_t>(off) = 0;
        cls.at<std::uint32_t>(off + 4) = 0;
        cls.at<std::uint32_t>(off + 8) = 0;
    };
    if (cls.ptr(0x5BC) != nullptr) host.operator_delete_00bf65ac(cls.ptr(0x5BC));
    zero3(0x5BC);
    release_ref(0x5A4);
    for (std::size_t vec : {std::size_t{0x560}, std::size_t{0x550}}) {
        if (cls.ptr(vec + 4) != nullptr) {
            host.stl_destroy_range_007cd3f0(cls.ptr(vec + 4), cls.ptr(vec + 8), cls.field(vec), 0);
            host.operator_delete_00bf65ac(cls.ptr(vec + 4));
        }
        zero3(vec + 4);
    }
    host.plane_engine_effect_vector_destroy_records_007ccd30(cls.sub(0x540));
    host.plane_engine_effect_vector_resize_007c9290(cls.sub(0x250), 0);
    host.free_00bf6989(cls.ptr(0x250));
    const Raw pods = cls.sub(0x238);
    if (pods.at<std::int32_t>(8) < 0) host.pod_array64_reserve_007bd600(pods, 0);
    while (pods.at<std::int32_t>(4) > 0) pods.at<std::int32_t>(4) -= 1;
    pods.at<std::int32_t>(4) = 0;
    host.free_00bf6989(pods.ptr(0));
    release_ref(0x234);
    if (cls.ptr(0x228) != nullptr) {
        host.reference_pointer_range_release_004cc760(cls.ptr(0x228), cls.ptr(0x22C), cls.field(0x224), 0);
        host.operator_delete_00bf65ac(cls.ptr(0x228));
    }
    zero3(0x228);
    release_ref(0x220);
    release_ref(0x218);
    release_ref(0x214);
    release_ref(0x210);
    if (cls.ptr(0x14C) != nullptr) host.operator_delete_00bf65ac(cls.ptr(0x14C));
    zero3(0x14C);
    native_string_release(host, cls.sub(0x138));
    host.vehicle_class_destruct_base_0095f1c0(cls);
}

// 007C75A0, __thiscall(ECX = plane), RET 0. Under the same lock: plane+C40h =
// 0; the first element equal to the plane in [data, data + size) is
// overwritten with the last element and size -= 1 (order not kept).
void plane_unregister_firing_guns_007c75a0(Seg47bHost& host, Raw plane) {
    void* const cs = raw(cc13_seg47::firing_gun_registry_instance_007c2ec0(host)).ptr(4);
    if (cs != nullptr) {
        host.enter_critical_section_ce2218(cs);
        raw(cs).at<std::int32_t>(0x18) += 1;
    }
    plane.at<std::uint8_t>(0xC40) = 0;
    std::uint32_t* const data = *static_cast<std::uint32_t**>(host.global_address(0x00F87278u));
    auto& size = *static_cast<std::uint32_t*>(host.global_address(0x00F8727Cu));
    std::uint32_t* const end = data + size;
    const auto self = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(plane.p));
    for (std::uint32_t* it = data; it != end; ++it) {
        if (*it == self) {
            *it = end[-1];
            size -= 1;
            break;
        }
    }
    if (cs != nullptr) {
        raw(cs).at<std::int32_t>(0x18) -= 1;
        host.leave_critical_section_ce2210(cs);
    }
}

}  // namespace bsp::cc13_seg47b
