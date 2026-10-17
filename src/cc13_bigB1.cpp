// cc13_bigB1: three large leftovers of the segment 15-51 sweep, each as a
// whole body (packet cc13-bigB1, docs/CC13_BIGB1.md). Names are hypotheses;
// see the header for the hooks.
//
// x87 note (rule 4c): these functions run on the main thread, whose x87
// precision control is 24 bits (docs/CC13_FPU_PRECISION.md), so every x87
// arithmetic operation is modelled as rounding to float in the image's order.
// An operation with a double constant is written pc24(double op double).
// SSE arithmetic is float as written. EH frames are noted, not modelled.

#include "bsp/cc13_bigB1.hpp"

#include "bsp/native_scalar_float_by_ref.hpp"
#include "bsp/profile_seg2_cc13.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/vector_helpers.hpp"

#include <cmath>
#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

// One x87 operation under 24-bit precision control: the exact result rounded
// to float.
float pc24(double value) {
    return static_cast<float>(value);
}

// MOVSS + AND 7FFFFFFFh: the sign bit cleared, NaN kept.
float abs_bits(float value) {
    return std::fabs(value);
}

std::uint8_t* bytes(void* base) {
    return static_cast<std::uint8_t*>(base);
}

}  // namespace

// ---------------------------------------------------------------------------
// 007A91A0 BSP_MovCam_Step
// ---------------------------------------------------------------------------
//
// __thiscall(ECX = camera+170h, float dt), RET 4; frame SUB ESP,0B0h plus
// EBX/EBP/ESI/EDI, no SEH. Every callee that takes the camera gets ECX =
// EBP-170h, the MovCam object, so this body is written camera-relative
// (image offset = EBP offset + 170h). Constants read from the PE:
// 00D7A24C 1.0f, 00D7A208 -0.0f (SUBSS from it negates), 00CEDCD0 double
// 0.78539818525314331, 00D7A218 0.0f, 00D7A328 double 4.0, 00D7A348 double
// 0.25, 00D7A2F0 0.1f, 00CE3800 0.5f, 00CE3CCC -1.57079637f, 00CE4D70 double
// 200.0, 00D7A220 double 100.0, 00D04FD0 2900.0f, 00CE7530 double 3000.0,
// 00D7A280 double 0.5, 00D7A268 double 9.999999747378752e-05, 00CE3D08 100.0f,
// 00CFA424 3000.0f.
// x87 DC forms in the body (tools/x87_dc_form_scan.py): 007A9E5F, 007AA6CC,
// 007AA720 (FMUL ST0,ST0, a square) and 007AA6DF (DC C9, ST1 = ST1*ST0: the
// arrival radius below is v*v/accel*0.5 and 0.5 stays on the stack for the
// mode-3 turn radius), 007AA8A4 (DC E9, ST1 = ST1-ST0: h - 100). No 00BD2F10
// draws.
void cc13_bigB1_movcam_step_007a91a0(void* camera, float dt, Cc13BigB1Host& host) {
    void* const c = camera;
    const float neg_zero = -0.0f;  // 00D7A208

    // 007A91AC..007A91F0: latch copy +51Ch..+537h -> +500h..+51Bh (REP MOVSD,
    // 7 dwords), then the edge bytes +51Ch..+51Fh and +534h..+536h are cleared.
    std::memcpy(bytes(c) + 0x500, bytes(c) + 0x51C, 7 * 4);
    at<std::uint8_t>(c, 0x51F) = 0;
    at<std::uint8_t>(c, 0x51E) = 0;
    at<std::uint8_t>(c, 0x51D) = 0;
    at<std::uint8_t>(c, 0x51C) = 0;
    at<std::uint8_t>(c, 0x536) = 0;
    at<std::uint8_t>(c, 0x535) = 0;
    at<std::uint8_t>(c, 0x534) = 0;
    // 007A91C3..007A920E: actions 82h/83h.
    if (at<std::uint8_t>(c, 0x518) != 0 || at<std::uint8_t>(c, 0x519) != 0) {
        host.movcam_007a84d0(camera, 1);
    }

    // 007A9213..007A9306: action 7Bh (latched +503h) once the timer +494h has
    // run out (COMISS 0,[+494h]; JC: a positive or NaN timer skips) in modes
    // 1/2: next deck point. Otherwise, with a secondary target +390h and a
    // stick input, modes 1/2 drop their look target.
    {
        const std::int32_t mode = at<std::int32_t>(c, 0x384);
        if (at<std::uint8_t>(c, 0x503) != 0 && at<float>(c, 0x494) <= 0.0f && (mode == 2 || mode == 1)) {
            at<float>(c, 0x494) = 1.0f;
            cc13_sweepB4_movcam_step_deck_walk_007a8da0(camera, host);
            if (at<std::int32_t>(c, 0x384) == 1) {
                const std::int32_t count = at<std::int32_t>(c, 0x388);
                if (count % 2 == 0) {
                    at<std::int32_t>(c, 0x388) = count + 1;
                }
            }
        } else if (at<std::int32_t>(c, 0x390) != 0) {
            // FLD |+510h|; FLD |+50Ch|; FCOMIP; JBE: the larger, ties and NaN to |+510h|.
            const float a = abs_bits(at<float>(c, 0x510));
            const float b = abs_bits(at<float>(c, 0x50C));
            const float larger = b > a ? b : a;
            if (larger > 0.0f) {
                const std::int32_t m = at<std::int32_t>(c, 0x384);
                if (m == 1 || m == 2) {
                    host.movcam_set_look_target_00797070(camera, nullptr, 0);
                }
            }
        }
    }

    // 007A9309..007A932D: actions that force mode 1 (+500h) or mode 2 (+501h).
    if (at<std::uint8_t>(c, 0x500) != 0) {
        at<std::int32_t>(c, 0x384) = 1;
        cc13_sweepB6_movcam_enter_mode_007a5110(camera, 0, host);
    } else if (at<std::uint8_t>(c, 0x501) != 0) {
        at<std::int32_t>(c, 0x384) = 2;
        cc13_sweepB6_movcam_enter_mode_007a5110(camera, 0, host);
    }

    // 007A9335..007A9389: action +51Ah toggles the vehicle-camera editor byte
    // +538h (only in mode 2) and shows or hides the overlay widget
    // [[+4D0h]+28h] (vtable +34h).
    if (at<std::uint8_t>(c, 0x51A) != 0) {
        at<std::uint8_t>(c, 0x538) = at<std::uint8_t>(c, 0x538) == 0 ? 1 : 0;
        if (at<std::int32_t>(c, 0x384) != 2) {
            at<std::uint8_t>(c, 0x538) = 0;
        }
        void* const widget = at<void*>(at<void*>(c, 0x4D0), 0x28);
        host.widget_vcall_34_set_visible(widget, at<std::uint8_t>(c, 0x538) != 0 ? 1 : 0);
    }

    // 007A938E..007A9557: mode 1 sub-mode counter +388h (even = targeted, odd
    // = free). Action +502h toggles between them; a stick input moves on.
    if (at<std::int32_t>(c, 0x384) == 1) {
        const std::int32_t count = at<std::int32_t>(c, 0x388);
        bool parity_step = false;
        if (count < 2) {
            if (at<std::uint8_t>(c, 0x502) != 0) {
                float aim[3];
                at<std::int32_t>(c, 0x388) = count + 2;
                at<float>(c, 0x498) = 1.0f;
                void* const d = host.movcam_aim_delta_00794910(camera, aim, at<void*>(c, 0x38C), 0.0f);
                const float length = raw_length_2d_00414c60(static_cast<const float*>(d));
                // FLD double 00CEDCD0; FCOMIP; JBE: closer than pi/4 snaps to 2.
                if (0.78539818525314331 > static_cast<double>(length)) {
                    at<std::int32_t>(c, 0x388) = 2;
                }
            } else {
                parity_step = true;
            }
        } else if (at<std::uint8_t>(c, 0x502) != 0) {
            at<std::int32_t>(c, 0x388) = 0;
            at<float>(c, 0x498) = 1.0f;
        } else {
            parity_step = true;
        }
        if (parity_step) {
            // 007A9434: signed remainder (AND 80000001h idiom).
            // Even: a look input with no stick input frees the camera (and
            // clears the follow target +3E4h). Odd: a stick input re-targets.
            const std::int32_t parity = count % 2;
            if (parity == 0 && at<float>(c, 0x498) <= 0.0f) {
                if (abs_bits(at<float>(c, 0x50C)) > 0.0f || abs_bits(at<float>(c, 0x510)) > 0.0f) {
                    if (abs_bits(at<float>(c, 0x504)) <= 0.0f && abs_bits(at<float>(c, 0x508)) <= 0.0f) {
                        at<std::int32_t>(c, 0x3E4) = 0;
                        at<std::int32_t>(c, 0x388) = count + 1;
                    }
                }
            }
            if (parity == 1 && at<float>(c, 0x494) <= 0.0f) {
                if (abs_bits(at<float>(c, 0x504)) > 0.0f || abs_bits(at<float>(c, 0x508)) > 0.0f) {
                    at<std::int32_t>(c, 0x388) = count - 1;
                }
            }
        }
    }

    // 007A955D: the editor's debug draw.
    if (at<std::uint8_t>(c, 0x538) != 0) {
        host.movcam_debug_draw_vehicle_cameras_007a01f0(camera);
    }

    // 007A9574..007A963F: interpolator list (checked std::list at +4D4h: +4D8h
    // head node, +4DCh size; payload at node+8h). Each record's clock +14h
    // advances by dt, 007901D0 applies it, and a record past +10h + +0Ch is
    // freed and unlinked. The 00BF6713 calls are the checked-iterator tests.
    {
        std::uint8_t* const list = bytes(c) + 0x4D4;
        std::uint8_t* node = at<std::uint8_t*>(at<std::uint8_t*>(c, 0x4D8), 0);
        for (;;) {
            std::uint8_t* const head = at<std::uint8_t*>(c, 0x4D8);
            if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            if (node == head) break;
            if (list == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
            if (node == at<std::uint8_t*>(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
            void* const record = at<void*>(node, 8);
            at<float>(record, 0x14) = at<float>(record, 0x14) + dt;
            host.movcam_apply_interpolator_007901d0(record, camera);
            const float end = at<float>(record, 0x10) + at<float>(record, 0x0C);
            if (at<float>(record, 0x14) > end) {
                host.crt_operator_delete_00bf65ac(record);
                if (node == at<std::uint8_t*>(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                std::uint8_t* const next = at<std::uint8_t*>(node, 0);
                if (node != at<std::uint8_t*>(c, 0x4D8)) {
                    at<std::uint8_t*>(at<std::uint8_t*>(node, 4), 0) = next;
                    at<std::uint8_t*>(at<std::uint8_t*>(node, 0), 4) = at<std::uint8_t*>(node, 4);
                    host.crt_operator_delete_00bf65ac(node);
                    at<std::int32_t>(c, 0x4DC) -= 1;
                }
                node = next;
            } else {
                if (node == at<std::uint8_t*>(list, 4)) host.crt_invalid_parameter_noinfo_00bf6713();
                node = at<std::uint8_t*>(node, 0);
            }
        }
    }

    // 007A9644..007A9699: accelerations cleared, both timers run down.
    at<float>(c, 0x3AC) = 0.0f;
    at<float>(c, 0x3B0) = 0.0f;
    at<float>(c, 0x3B4) = 0.0f;
    at<float>(c, 0x3D4) = 0.0f;
    at<float>(c, 0x3D8) = 0.0f;
    at<float>(c, 0x494) = at<float>(c, 0x494) - dt;
    at<float>(c, 0x498) = at<float>(c, 0x498) - dt;

    // 007A969F..007A9704: reference velocity +3B8h = the target's velocity
    // (target vtable +34h) unless there is no target, the game byte
    // [00E188A8]+634h is set, the mode is 2, or mode 1 is in sub-mode >= 2.
    {
        void* const target = at<void*>(c, 0x38C);
        const std::int32_t mode = at<std::int32_t>(c, 0x384);
        if (target != nullptr && at<std::uint8_t>(host.global_00e188a8(), 0x634) == 0 && mode != 2 &&
            !(mode == 1 && at<std::int32_t>(c, 0x388) >= 2)) {
            float velocity[3];
            const float* const v = host.unit_vcall_34_velocity(target, velocity);
            at<float>(c, 0x3B8) = v[0];
            at<float>(c, 0x3BC) = v[1];
            at<float>(c, 0x3C0) = v[2];
        } else {
            at<float>(c, 0x3B8) = 0.0f;
            at<float>(c, 0x3BC) = 0.0f;
            at<float>(c, 0x3C0) = 0.0f;
        }
    }

    // 007A970C..007A98BB: reference turn rates +3DCh/+3E0h: the relative
    // velocity projected on the look basis rows 1 and 0, over the distance to
    // the look entity (the secondary target, else the target in mode 1 even
    // or mode 4).
    {
        void* look = at<void*>(c, 0x390);
        bool run = look != nullptr;
        if (!run) {
            look = at<void*>(c, 0x38C);
            const std::int32_t mode = at<std::int32_t>(c, 0x384);
            run = look != nullptr &&
                  ((mode == 1 && at<std::int32_t>(c, 0x388) % 2 == 0) || mode == 4);
        }
        if (run) {
            float m[16];
            host.movcam_look_basis_00795300(camera, m, look);
            if (at<std::uint8_t>(look, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(look);
            float d[3];
            d[0] = at<float>(look, 0xFC) - at<float>(c, 0x394);
            d[1] = at<float>(look, 0x100) - at<float>(c, 0x398);
            d[2] = at<float>(look, 0x104) - at<float>(c, 0x39C);
            // FLD1; FDIVRP: 1 / length.
            const float inv = pc24(1.0 / host.vector3_length_0042b2f0(d));
            float v0 = at<float>(c, 0x3A0) - at<float>(c, 0x3B8);
            float v1 = at<float>(c, 0x3A4) - at<float>(c, 0x3BC);
            float v2 = at<float>(c, 0x3A8) - at<float>(c, 0x3C0);
            float t0 = m[0] * v0;
            float t1 = m[1] * v1;
            float t2 = m[2] * v2;
            const float dot0 = (t0 + t1) + t2;
            v0 = at<float>(c, 0x3A0) - at<float>(c, 0x3B8);
            v1 = at<float>(c, 0x3A4) - at<float>(c, 0x3BC);
            v2 = at<float>(c, 0x3A8) - at<float>(c, 0x3C0);
            t0 = m[4] * v0;
            t1 = m[5] * v1;
            t2 = m[6] * v2;
            const float dot1 = (t0 + t1) + t2;
            const float rate_x = -dot1 * inv;
            const float rate_y = inv * dot0;
            at<float>(c, 0x3DC) = rate_x;
            at<float>(c, 0x3E0) = rate_y;
        } else {
            at<float>(c, 0x3DC) = 0.0f;
            at<float>(c, 0x3E0) = 0.0f;
        }
    }

    void* const settings = at<void*>(c, 0x4D0);
    const std::int32_t mode = at<std::int32_t>(c, 0x384);

    // Steer along the mover's forward row +94h..+9Ch scaled by the throttle
    // input +514h (modes 0 and 1 odd).
    const auto steer_forward = [&] {
        const float s = at<float>(c, 0x514);
        const float x = s * at<float>(c, 0x94);
        const float y = at<float>(c, 0x98) * s;
        const float z = at<float>(c, 0x9C) * s;
        host.movcam_steer_position_00794b80(camera, x, y, z, 1.0f, 1);
    };

    if (mode == 0) {
        // 007A98D1..007A997A: free camera.
        host.movcam_steer_angles_00794d60(camera, at<float>(c, 0x510), neg_zero - at<float>(c, 0x50C), 1.0f, 1,
                                          nullptr);
        steer_forward();
        host.movcam_integrate_007943d0(camera, dt);
        host.movcam_keep_above_ground_00790540(camera);
    } else if (mode == 1) {
        // 007A997F..007AA061: targeted (even) and free (odd) sub-modes.
        void* target = at<void*>(c, 0x38C);
        if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
        float d[3];
        d[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
        d[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
        d[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
        const float distance = static_cast<float>(host.vector3_length_0042b2f0(d));
        target = at<void*>(c, 0x38C);  // 007A99EB, re-read after the call
        // [[target+538h]+A0h]: the target's size; ratio = distance/(size*4)+0.25, capped at 1.
        const float size4 = pc24(static_cast<double>(at<float>(at<void*>(target, 0x538), 0xA0)) * 4.0);
        const float ratio = pc24(static_cast<double>(distance / size4) + 0.25);
        const float k = ratio > 1.0f ? 1.0f : ratio;
        if (at<std::int32_t>(c, 0x388) % 2 == 0) {
            // FLD |+508h|; FLD |+504h|; FCOMIP; JBE: the larger, ties and NaN to |+508h|.
            const float a = abs_bits(at<float>(c, 0x508));
            const float b = abs_bits(at<float>(c, 0x504));
            const float larger = b > a ? b : a;
            const bool stick = larger > 0.0f;  // COMISS 00D7A218 (0.0f); JA
            float aim[3];
            host.movcam_aim_delta_00794910(camera, aim, target, at<float>(settings, 0x54));
            host.movcam_steer_angles_braking_00794f70(camera, neg_zero - aim[0], neg_zero - aim[1],
                                                      at<void*>(c, 0x38C));
            if (stick) {
                // 007A9B0D..007A9C9A: orbit with the stick in the look basis.
                float m[16];
                host.movcam_look_basis_00795300(camera, m, at<void*>(c, 0x38C));
                target = at<void*>(c, 0x38C);
                if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
                float v[3];
                v[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
                v[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
                v[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
                const float s = at<float>(c, 0x514);
                float nb[3];
                const float* const n = host.vector3_normalize_00419510(nb, v);
                const float a0 = n[0] * s;
                const float a1 = n[1] * s;
                const float a2 = s * n[2];
                const float ny = -at<float>(c, 0x508);
                const float b0 = m[4] * ny;
                const float b1 = m[5] * ny;
                const float b2 = ny * m[6];
                const float nx = -at<float>(c, 0x504);
                const float c0 = m[0] * nx;
                const float c1 = m[1] * nx;
                const float c2 = nx * m[2];
                const float e0 = (c0 + b0) + a0;
                const float e1 = (c1 + b1) + a1;
                const float e2 = (c2 + b2) + a2;
                host.movcam_steer_position_00794b80(camera, e0 * k, e1 * k, k * e2, 1.0f, 1);
            } else {
                // 007A9C9F..007A9D5F: straight at the target.
                target = at<void*>(c, 0x38C);
                if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
                float v[3];
                v[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
                v[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
                v[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
                const float s = at<float>(c, 0x514);
                float nb[3];
                const float* const n = host.vector3_normalize_00419510(nb, v);
                const float a0 = n[0] * s;
                const float a1 = n[1] * s;
                const float a2 = s * n[2];
                host.movcam_steer_position_00794b80(camera, a0 * k, a1 * k, k * a2, 1.0f, 1);
            }
            // 007A9D64..007A9F58: keep the orbit: the part of the next-step
            // relative velocity across the line of sight, |q x n|^2/|d|, is added
            // to the acceleration toward the target.
            target = at<void*>(c, 0x38C);
            if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
            float p[3];
            p[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
            p[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
            p[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
            const float g0 = at<float>(c, 0x3AC) * dt;
            const float g1 = at<float>(c, 0x3B0) * dt;
            const float g2 = dt * at<float>(c, 0x3B4);
            const float h0 = at<float>(c, 0x3A0) - at<float>(c, 0x3B8);
            const float h1 = at<float>(c, 0x3A4) - at<float>(c, 0x3BC);
            const float h2 = at<float>(c, 0x3A8) - at<float>(c, 0x3C0);
            float q[3] = {h0 + g0, h1 + g1, h2 + g2};
            float nb[3];
            float* const n = host.vector3_normalize_00419510(nb, p);
            float cb[3];
            const float* const cr = camera_vector_cross_004f9b30(cb, q, n);
            const float cc0 = cr[0] * cr[0];
            const float cc1 = cr[1] * cr[1];
            const float cc2 = cr[2] * cr[2];
            const float cross_sq = (cc0 + cc1) + cc2;
            target = at<void*>(c, 0x38C);
            if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
            float d2[3];
            d2[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
            d2[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
            d2[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
            target = at<void*>(c, 0x38C);
            if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);
            float d3[3];
            d3[0] = at<float>(target, 0xFC) - at<float>(c, 0x394);
            d3[1] = at<float>(target, 0x100) - at<float>(c, 0x398);
            d3[2] = at<float>(target, 0x104) - at<float>(c, 0x39C);
            // FSTP double; 0042B2F0; FDIVR double: cross_sq / |d2|.
            const float centripetal = pc24(static_cast<double>(cross_sq) / host.vector3_length_0042b2f0(d2));
            float nb2[3];
            const float* const n2 = host.vector3_normalize_00419510(nb2, d3);
            const float r0 = n2[0] * centripetal;
            const float r1 = n2[1] * centripetal;
            const float r2 = centripetal * n2[2];
            at<float>(c, 0x3AC) = at<float>(c, 0x3AC) + r0;
            at<float>(c, 0x3B0) = r1 + at<float>(c, 0x3B0);
            at<float>(c, 0x3B4) = r2 + at<float>(c, 0x3B4);
        } else {
            // 007A9F5D..007AA02B: free flight, looking at the secondary target if any.
            void* const secondary = at<void*>(c, 0x390);
            if (secondary != nullptr) {
                float aim[3];
                const float* const o = cc13_sweepB_movcam_aim_delta_default_00794b50(camera, aim, secondary, host);
                host.movcam_steer_angles_00794d60(camera, neg_zero - o[0], neg_zero - o[1], 1.0f, 0, secondary);
            } else {
                host.movcam_steer_angles_00794d60(camera, at<float>(c, 0x510), neg_zero - at<float>(c, 0x50C), 1.0f,
                                                  1, nullptr);
            }
            steer_forward();
        }
        // 007AA030..007AA05C
        host.movcam_integrate_007943d0(camera, dt);
        host.movcam_keep_above_ground_00790540(camera);
        host.movcam_check_secondary_arrival_0079da90(camera, at<void*>(c, 0x390), 0.1f);
    } else if (mode == 2) {
        // 007AA06F..007AA154: deck mode; the overlay colour is yellow, or with
        // the editor open green/red for inside/outside the vehicle camera's
        // angle windows (element +48h) at the angles +3C8h/+3C4h.
        void* const target = at<void*>(c, 0x38C);
        if (at<std::uint8_t>(c, 0x538) != 0 && at<void*>(at<void*>(target, 0x538), 0xBC) != nullptr) {
            const float angle_3c4 = at<float>(c, 0x3C4);
            const float angle_3c8 = at<float>(c, 0x3C8);
            void* const element = host.vehicle_camera_array_at_005cd820(at<void*>(at<void*>(target, 0x538), 0xBC),
                                                                         at<std::int32_t>(c, 0x388));
            const bool inside = host.movcam_angle_windows_contain_0078e2b0(bytes(element) + 0x48, angle_3c8,
                                                                           angle_3c4);
            if (inside) {
                const float green[4] = {0.0f, 1.0f, 0.0f, 1.0f};
                host.widget_vcall_50_colour(at<void*>(at<void*>(c, 0x4D0), 0x28), green);
            } else {
                const float red[4] = {1.0f, 0.0f, 0.0f, 1.0f};
                host.widget_vcall_50_colour(at<void*>(at<void*>(c, 0x4D0), 0x28), red);
            }
        } else {
            const float yellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
            host.widget_vcall_50_colour(at<void*>(at<void*>(c, 0x4D0), 0x28), yellow);
        }
        // 007AA156..007AA23E: a stick push over 0.5 (latched in +490h until
        // released) opens the next vehicle camera when the target has more
        // than one: 0079F9E0 with the editor open, else 007A87B0.
        if (abs_bits(at<float>(c, 0x504)) <= 0.0f && abs_bits(at<float>(c, 0x508)) <= 0.0f) {
            at<std::uint8_t>(c, 0x490) = 0;
        } else if (at<std::uint8_t>(c, 0x490) == 0) {
            if (abs_bits(at<float>(c, 0x504)) > 0.5f || abs_bits(at<float>(c, 0x508)) > 0.5f) {
                void* const cameras = at<void*>(at<void*>(at<void*>(c, 0x38C), 0x538), 0xBC);
                if (cameras != nullptr && at<std::int32_t>(cameras, 4) > 1) {
                    at<std::uint8_t>(c, 0x490) = 1;
                    if (at<std::uint8_t>(c, 0x538) != 0) {
                        host.movcam_edit_vehicle_camera_window_0079f9e0(camera);
                    } else {
                        host.movcam_update_hand_mode_007a87b0(camera);
                    }
                }
            }
        }
        // 007AA243..007AA318: turn toward the secondary target, else by the stick,
        // both divided by the zoom +460h.
        void* const secondary = at<void*>(c, 0x390);
        if (secondary != nullptr) {
            if (!host.movcam_has_vehicle_camera_pose_007a0dd0(camera, secondary, at<std::int32_t>(c, 0x388))) {
                cc13_sweepB2_movcam_pick_deck_point_scored_007a8c60(camera, at<void*>(c, 0x390), host);
            }
            float aim[3];
            const float* const o = cc13_sweepB_movcam_aim_delta_default_00794b50(camera, aim, at<void*>(c, 0x390), host);
            const float ox = -o[0];
            void* const look = at<void*>(c, 0x390);
            const float oy = -o[1];
            const float zoom = at<float>(c, 0x460);
            host.movcam_steer_angles_00794d60(camera, ox / zoom, oy / zoom, 1.0f, 0, look);
        } else {
            const float ny = -at<float>(c, 0x50C);
            const float nx = at<float>(c, 0x510);
            const float zoom = at<float>(c, 0x460);
            host.movcam_steer_angles_00794d60(camera, nx / zoom, ny / zoom, 1.0f, 1, nullptr);
        }
        host.movcam_integrate_007943d0(camera, dt);
        // 007AA32F..007AA394: remember the angles per (target, vehicle camera)
        // in the map [+4D0h]+70h, pin the position to the deck point +A4h, and
        // check the secondary arrival.
        const std::int32_t index = at<std::int32_t>(c, 0x388);
        void* const record = host.map_subscript_007a4e10(bytes(at<void*>(c, 0x4D0)) + 0x70, bytes(c) + 0x38C);
        float* const pair = host.vector8_at_00418170(record, static_cast<std::uint32_t>(index));
        pair[0] = at<float>(c, 0x3C4);
        pair[1] = at<float>(c, 0x3C8);
        at<float>(c, 0x394) = at<float>(c, 0xA4);
        at<float>(c, 0x398) = at<float>(c, 0xA8);
        at<float>(c, 0x39C) = at<float>(c, 0xAC);
        host.movcam_check_secondary_arrival_0079da90(camera, at<void*>(c, 0x390), 0.1f);
    } else if (mode == 3 || mode == 4 || mode == 6) {
        if (mode == 6) {
            // 007AA531..007AA6AD: the reference point +408h: the target's
            // frame row 2 scaled by +488h and [+4D0h]+54h over the zoom, added to
            // its position, pushed +488h further away from the secondary
            // target, and raised by +48Ch.
            const float* const w = host.entity_get_world_matrix_0042d7e0(at<void*>(c, 0x38C));
            const float a = at<float>(c, 0x488);
            const float t0 = a * w[8];
            const float t1 = w[9] * a;
            const float t2 = a * w[10];
            const float lead = at<float>(at<void*>(c, 0x4D0), 0x54);
            const float u0 = t0 * lead;
            const float u1 = t1 * lead;
            const float u2 = lead * t2;
            const float zoom = at<float>(c, 0x460);
            const float w0 = u0 / zoom;
            const float w1 = u1 / zoom;
            const float w2 = u2 / zoom;
            const float* const p = host.entity_world_position_00427eb0(at<void*>(c, 0x38C));
            const float a0 = p[0] + w0;
            const float a1 = p[1] + w1;
            const float a2 = p[2] + w2;
            const float* const q = host.entity_world_position_00427eb0(at<void*>(c, 0x390));
            float d[3] = {a0 - q[0], a1 - q[1], a2 - q[2]};
            float nb[3];
            const float* const n = host.vector3_normalize_00419510(nb, d);
            const float r = at<float>(c, 0x488);
            const float m0 = n[0] * r;
            const float m1 = n[1] * r;
            const float m2 = r * n[2];
            const float b0 = m0 + a0;
            const float b1 = m1 + a1;
            const float b2 = m2 + a2;
            at<float>(c, 0x408) = b0;
            at<float>(c, 0x40C) = b1;
            at<float>(c, 0x410) = b2;
            at<float>(c, 0x40C) = at<float>(c, 0x48C) + at<float>(c, 0x40C);
        }
        // 007AA6B3..007AA6EF: arrival radius R = top^2/accel * 0.5 ([+4D0h]
        // +34h, +4Ch); the look entity is the secondary target, else the target.
        void* const limits = at<void*>(c, 0x4D0);
        const float top = at<float>(limits, 0x34);
        const float top_sq = top * top;
        const float stopping = top_sq / at<float>(limits, 0x4C);
        const float radius = pc24(static_cast<double>(stopping) * 0.5);
        void* const look = at<void*>(c, 0x390) != nullptr ? at<void*>(c, 0x390) : at<void*>(c, 0x38C);
        if (at<std::int32_t>(c, 0x384) == 3) {
            // 007AA703..007AA787: overview: pitch -pi/2, yaw by the stick; the
            // turn radius is turn_top^2/turn_accel * 0.5 with the stick's sign.
            const float turn_top = at<float>(limits, 0x38);
            const float turn_sq = turn_top * turn_top;
            const float turn_radius = pc24(0.5 * static_cast<double>(turn_sq / at<float>(limits, 0x50)));
            host.movcam_steer_angles_00794d60(camera, -1.57079637f, at<float>(c, 0x50C), 1.0f, 0, nullptr);
            const std::int32_t sign = host.math_sign_float_0064a740(at<float>(c, 0x50C));
            at<float>(c, 0x418) = -1.57079637f;
            at<float>(c, 0x41C) = static_cast<float>(sign) * turn_radius;
        } else if (look != nullptr) {
            // 007AA792..007AA7EC: turn toward the look entity and store the offset.
            float aim[3];
            cc13_sweepB_movcam_aim_delta_default_00794b50(camera, aim, look, host);
            host.movcam_steer_angles_00794d60(camera, neg_zero - aim[0], neg_zero - aim[1], 1.0f, 0, look);
            at<float>(c, 0x41C) = at<float>(c, 0x3C8) - aim[1];
            at<float>(c, 0x418) = at<float>(c, 0x3C4) - aim[0];
        } else {
            // 007AA7F4..007AA830: keep the stored angles +418h/+41Ch.
            const float dx = at<float>(c, 0x418) - at<float>(c, 0x3C4);
            const float dy = at<float>(c, 0x41C) - at<float>(c, 0x3C8);
            host.movcam_steer_angles_00794d60(camera, dx, dy, 1.0f, 0, nullptr);
        }

        void* const ref = bytes(c) + 0x3F0;
        // UCOMISS + LAHF/TEST AH,44h/JP: "!= 0", NaN counts as non-zero.
        if (at<std::int32_t>(c, 0x384) == 3 &&
            (at<float>(c, 0x514) != 0.0f || at<float>(c, 0x504) != 0.0f || at<float>(c, 0x508) != 0.0f)) {
            // 007AA87B..007AAB3D: overview with input: fly in the mover basis
            // (+74h right, +84h up, +94h forward rows), the throttle scaled down
            // near the height bounds 100/3000, then brake the vertical
            // acceleration at those bounds and move the reference +3F0h to the
            // stopping point.
            const float throttle = at<float>(c, 0x514);
            const float height = at<float>(c, 0x40C);
            float factor = throttle;
            if (200.0 > static_cast<double>(height) && throttle > 0.0f) {
                float zero = 0.0f;
                float value = pc24(static_cast<double>(pc24(static_cast<double>(height) - 100.0)) / 100.0);
                factor = max_native_float_by_ref_00415550(&zero, &value);
            } else if (height > 2900.0f && 0.0f > throttle) {
                float zero = 0.0f;
                float value = pc24(static_cast<double>(pc24(static_cast<double>(height) - 3000.0)) / 100.0);
                factor = min_native_float_by_ref_00415510(&zero, &value);
            }
            const float f0 = at<float>(c, 0x94) * factor;
            const float f1 = at<float>(c, 0x98) * factor;
            const float f2 = factor * at<float>(c, 0x9C);
            const float ny = -at<float>(c, 0x508);
            const float u0 = ny * at<float>(c, 0x84);
            const float u1 = at<float>(c, 0x88) * ny;
            const float u2 = ny * at<float>(c, 0x8C);
            const float x = at<float>(c, 0x504);
            const float r0 = x * at<float>(c, 0x74);
            const float r1 = at<float>(c, 0x78) * x;
            const float r2 = x * at<float>(c, 0x7C);
            const float s0 = r0 + u0;
            const float s1 = r1 + u1;
            const float s2 = r2 + u2;
            host.movcam_steer_position_00794b80(camera, s0 + f0, s1 + f1, s2 + f2, 1.0f, 0);

            float* const velocity = &at<float>(c, 0x3A0);
            const float speed = static_cast<float>(host.vector3_length_0042b2f0(velocity));
            const float accel = at<float>(at<void*>(c, 0x4D0), 0x4C);
            const float p0 = speed * velocity[0];
            const float p1 = velocity[1] * speed;
            const float p2 = speed * velocity[2];
            const float q0 = p0 / accel;
            const float q1 = p1 / accel;
            const float q2 = p2 / accel;
            float stop[3];
            stop[0] = at<float>(c, 0x394) + q0;
            stop[1] = at<float>(c, 0x398) + q1;
            stop[2] = at<float>(c, 0x39C) + q2;
            float height_out;
            if (100.0 > static_cast<double>(stop[1])) {
                float one = 1.0f;
                float value = pc24(100.0 - static_cast<double>(stop[1]));
                const float m = min_native_float_by_ref_00415510(&one, &value);
                height_out = 100.0f;  // 00CE3D08
                stop[1] = height_out;
                at<float>(c, 0x3B0) = m * accel;
            } else if (stop[1] > 3000.0f) {
                float one = 1.0f;
                float value = pc24(static_cast<double>(stop[1]) - 3000.0);
                const float m = min_native_float_by_ref_00415510(&one, &value);
                height_out = 3000.0f;  // 00CFA424
                stop[1] = height_out;
                at<float>(c, 0x3B0) = -m * accel;
            } else {
                height_out = stop[1];
            }
            at<float>(at<void*>(host.global_00e198c4(), 0x98), 0x5C) = height_out;
            host.camera_ref_set_point_00793cd0(ref, stop, 1);
        } else {
            // 007AAB47..007AAD44: approach the reference +3F0h with the speed
            // cap R*4; beyond R, a velocity turning away (v.a < -accel*top/4)
            // has its acceleration bent across the velocity at the same size.
            const float max_speed = pc24(static_cast<double>(radius) * 4.0);
            float rb[3];
            const float* const r = host.camera_ref_resolve_00793ac0(ref, rb, 1);
            const float dy = r[1] - at<float>(c, 0x398);
            const float dz = r[2] - at<float>(c, 0x39C);
            const float dx = r[0] - at<float>(c, 0x394);
            host.movcam_steer_position_00794b80(camera, dx, dy, dz, max_speed, 0);
            float rb2[3];
            const float* const r2 = host.camera_ref_resolve_00793ac0(ref, rb2, 1);
            float d[3];
            d[0] = r2[0] - at<float>(c, 0x394);
            d[1] = r2[1] - at<float>(c, 0x398);
            d[2] = r2[2] - at<float>(c, 0x39C);
            if (host.vector3_length_0042b2f0(d) > static_cast<double>(radius)) {
                float* const velocity = &at<float>(c, 0x3A0);
                float* const accel_vec = &at<float>(c, 0x3AC);
                const float along = profile_seg2_cc13::vector3_dot_0042b220(velocity, accel_vec);
                void* const lim = at<void*>(c, 0x4D0);
                const float product = at<float>(lim, 0x4C) * at<float>(lim, 0x34);
                const float threshold = -pc24(static_cast<double>(product) * 0.25);
                if (threshold > along) {
                    const float size = static_cast<float>(host.vector3_length_0042b2f0(accel_vec));
                    float nb[3];
                    const float* const n = host.vector3_normalize_00419510(nb, velocity);
                    const float k1 = accel_vec[1] * n[1];
                    const float k0 = accel_vec[0] * n[0];
                    const float k2 = accel_vec[2] * n[2];
                    const float proj_len = (k0 + k1) + k2;
                    const float n0 = n[0];
                    const float n1 = n[1];
                    const float n2 = n[2];
                    nb[0] = n0 * proj_len;
                    nb[1] = n1 * proj_len;
                    nb[2] = proj_len * n2;
                    profile_seg2_cc13::vector3_subtract_in_place_0042aee0(accel_vec, nb);
                    float mb[3];
                    if (host.vector3_length_0042b2f0(accel_vec) > 9.999999747378752e-05) {
                        const float* const m = host.vector3_normalize_00419510(mb, accel_vec);
                        const float x0 = size * m[0];
                        const float x1 = m[1] * size;
                        const float x2 = size * m[2];
                        accel_vec[0] = x0;
                        accel_vec[1] = x1;
                        accel_vec[2] = x2;
                    } else {
                        const float* const m = host.vector3_normalize_00419510(mb, velocity);
                        const float g0 = -m[0];
                        const float g1 = -m[1];
                        const float g2 = -m[2];
                        const float x0 = size * g0;
                        const float x1 = size * g1;
                        const float x2 = size * g2;
                        accel_vec[0] = x0;
                        accel_vec[1] = x1;
                        accel_vec[2] = x2;
                    }
                }
            }
        }
        // 007AAD47..007AAD91
        host.movcam_integrate_007943d0(camera, dt);
        host.movcam_keep_above_ground_00790540(camera);
        host.movcam_check_secondary_arrival_0079da90(camera, at<void*>(c, 0x390), 0.1f);
        // 00793AC0 resolves into the argument slots of 0079D960.
        float goal[3];
        host.camera_ref_resolve_00793ac0(ref, goal, 1);
        host.movcam_check_arrival_0079d960(camera, goal[0], goal[1], goal[2], radius);
    } else if (mode == 5) {
        // 007AA3C6..007AA47A: path mode: 007972F0 with a knot path (+474h),
        // else aim at position + sample(+464h path, t +468h); arrival (1.0)
        // once |+46Ch - +468h| is under the top speed.
        if (at<std::int32_t>(c, 0x474) != 0) {
            host.camera_position_update_path_007972f0(camera);
        } else {
            float sample[3];
            host.camera_path_sample_world_007b04c0(at<void*>(c, 0x464), at<float>(c, 0x468), &at<float>(c, 0x394),
                                                   sample, 0);
            float sum[3];
            host.vector3_add_to_004142a0(&at<float>(c, 0x394), sum, sample);
            host.camera_position_aim_at_point_00794070(camera, sum[0], sum[1], sum[2]);
        }
        const float gap = abs_bits(at<float>(c, 0x46C) - at<float>(c, 0x468));
        if (at<float>(at<void*>(c, 0x4D0), 0x34) > gap) {
            host.movcam_check_arrival_0079d960(camera, at<float>(c, 0x394), at<float>(c, 0x398), at<float>(c, 0x39C),
                                               1.0f);
        }
    } else if (mode == 7) {
        // 007AA488..007AA51E: position = +4C4h, look point = +4B8h, each in its
        // entity's frame (target / secondary target) when there is one.
        void* const target = at<void*>(c, 0x38C);
        float pb[3];
        const float* p = &at<float>(c, 0x4C4);
        if (target != nullptr) {
            const float* const w = host.entity_get_world_matrix_0042d7e0(target);
            p = host.vector3_transform_affine_point_via_scratch_00414d10(pb, &at<float>(c, 0x4C4), w);
        }
        at<float>(c, 0x394) = p[0];
        at<float>(c, 0x398) = p[1];
        at<float>(c, 0x39C) = p[2];
        void* const secondary = at<void*>(c, 0x390);
        float lb[3];
        const float* l = &at<float>(c, 0x4B8);
        if (secondary != nullptr) {
            const float* const w = host.entity_get_world_matrix_0042d7e0(secondary);
            l = host.vector3_transform_affine_point_via_scratch_00414d10(lb, &at<float>(c, 0x4B8), w);
        }
        host.camera_position_aim_at_point_00794070(camera, l[0], l[1], l[2]);
        host.movcam_keep_above_ground_00790540(camera);
    }

    // 007AAD96: the sub-object's own vtable +0Ch (ECX = camera+170h).
    host.movcam_controller_vcall_0c(bytes(c) + 0x170);
}

// ---------------------------------------------------------------------------
// 0060ABD0 BSP_HudPlaneEffectsScreen_Update
// ---------------------------------------------------------------------------
//
// __thiscall(ECX = screen, float dt), RET 4. SEH frame (handler 00C77557,
// state at [ESP0+1D0h]: 0 / 1 / 2 / 4 around the effect references, 3 around
// the stack hit record); unwinding is noted, not modelled. Constants from the
// PE: 00D7A24C 1.0f, 00D7A208 -0.0f, 00D7A238 0.01f, 00D7A300 -0.01f,
// 00CF01D0 double 5000.0, 00CE3938 double 50.0, 00CF48A8 double 2.412,
// 00CE4D70 double 200.0, 00CE685C 40.0f, 00D7A264 pi (f), 00CE684C -pi (f),
// 00CE3C64 pi/2 (f), 00CE3CCC -pi/2 (f), 00CE3D28 double pi, 00CE3830 double
// pi/2, 00CF48A0 double -pi/2, 00CEC9E0 double -0.5, 00D7A280 double 0.5,
// 00CE69D0 -0.5f, 00D7A260 -1.0f, 00CE3850 5.0f, 00CE3E18 0.7f, 00CEF290
// double 0.0625, 00D7A218 0.0f. x87 DC forms (x87_dc_form_scan): 0060B133 and
// 0060B1A2 (DC C9: the row-2 x term is ST1*5000 / ST1*50 and the constant stays
// for y and z), 0060BCEC (square), 0060BF35 (FADD ST0,ST0: thr+thr).
// The "-(p != 0) & 0E186ECh" tests on effect slots are p != 0.
void cc13_bigB1_hud_plane_effects_update_0060abd0(void* screen, float dt, Cc13BigB1Host& host) {
    void* const s = screen;
    const float neg_zero = -0.0f;  // 00D7A208
    void* const game = host.global_00e188a8();

    // Effect slot helpers (the three effect references +64h, +6Ch, +70h).
    const auto stop_effect = [&](std::size_t slot) {
        void* const effect = at<void*>(s, slot);
        if (effect != nullptr) {
            host.point_effect_stop_children_00867b10(effect);
            at<std::uint8_t>(effect, 9) = 1;
            host.effect_reference_adopt_without_retain_00484620(bytes(s) + slot, nullptr);
        }
    };
    // 00440490 copy-constructs the definition reference into the by-value
    // argument slot, 008687C0 creates the effect into a stack reference, which
    // 004845D0 assigns to the screen slot before 00440A30 releases it.
    const auto create_effect = [&](std::size_t slot, std::size_t definition, void* parent, const float* matrix) {
        void* definition_arg = nullptr;
        host.retained_reference_copy_construct_00440490(&definition_arg, bytes(s) + definition);
        void* created = nullptr;  // [ESP0+10h]
        void* const result =
            host.point_effect_create_with_parent_matrix_008687c0(&created, parent, definition_arg, matrix, 1, 0, 0);
        host.retained_reference_assign_004845d0(bytes(s) + slot, result);
        host.ref_ptr_slot_release_00440a30(&created);
    };

    // 0060ABF0..0060AC0A: no unit -> the HUD root's forced selection.
    if (at<void*>(s, 0xB8) == nullptr) {
        host.ingame_hud_force_select_unit_006485a0(at<void*>(host.global_00e198c4(), 0x40));
        return;
    }
    host.plane_step_countdown_007c3cb0(at<void*>(s, 0xB8), dt);
    if (at<void*>(s, 0x98) != nullptr && host.unit_vcall_1c0(at<void*>(s, 0xB8)) != 0) {
        host.plane_sync_cockpit_camera_gun_shake_007c7800(at<void*>(s, 0xB8), at<void*>(s, 0x98));
    }
    // 0060AC4D..0060ACB0: local player with [00E188A8]+19C4h set: clear the two
    // axis filters +E8h/+ECh and hand the role over (0077C470(2, 0)).
    if (host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 1) &&
        at<std::uint8_t>(host.global_00e188a8(), 0x19C4) != 0) {
        void* const f0 = at<void*>(s, 0xE8);
        at<std::uint8_t>(f0, 0) = 0;
        at<float>(f0, 8) = 0.0f;
        at<float>(f0, 0xC) = 0.0f;
        at<float>(f0, 0x10) = 0.0f;
        at<float>(f0, 4) = 0.0f;
        void* const f1 = at<void*>(s, 0xEC);
        at<std::uint8_t>(f1, 0) = 0;
        at<float>(f1, 8) = 0.0f;
        at<float>(f1, 0xC) = 0.0f;
        at<float>(f1, 0x10) = 0.0f;
        at<float>(f1, 4) = 0.0f;
        host.unit_0077c470(at<void*>(s, 0xB8), 2, 0);
    }
    // 0060ACB5..0060ACF3: no local input: spectate the unit's command object and
    // push interface request 24h.
    if (!host.plane_is_local_input_enabled_007bb9a0(at<void*>(s, 0xB8))) {
        host.ingame_hud_set_spectated_unit_00647300(at<void*>(host.global_00e198c4(), 0x40),
                                                    at<void*>(at<void*>(s, 0xB8), 0x9D4));
        host.frontend_push_interface_request_004cc460(host.global_00e198c4(), 0x24, at<void*>(s, 0xB8));
        return;
    }
    // 0060ACF8..0060AD5A: flight states 7/6/3, or 4 with +904h clear, run the
    // frame; otherwise, with +BF8h and +BF4h, spectate [[+BF4h]+4]+7Ch if it is
    // selectable, else force the selection.
    {
        void* const unit = at<void*>(s, 0xB8);
        const std::int32_t state = at<std::int32_t>(unit, 0x900);
        if (!(state == 7 || state == 6 || state == 3 || (state == 4 && at<std::uint8_t>(unit, 0x904) == 0))) {
            if (at<std::uint8_t>(unit, 0xBF8) == 0) return;
            void* const holder = at<void*>(unit, 0xBF4);
            if (holder == nullptr) return;
            void* const other = at<void*>(at<void*>(holder, 4), 0x7C);
            if (!host.unit_is_selectable_for_current_player_00645160(other, 1)) {
                host.ingame_hud_force_select_unit_006485a0(at<void*>(host.global_00e198c4(), 0x40));
                return;
            }
            host.ingame_hud_set_spectated_unit_00647300(at<void*>(host.global_00e198c4(), 0x40), other);
            return;
        }
    }
    const std::uint8_t flag_c24 = at<std::uint8_t>(at<void*>(s, 0xB8), 0xC24);  // [ESP0+34h], low byte

    // 0060AD69..0060ADB2: action DFh toggles the view mode +FCh.
    if (host.input_action_was_pressed_004c43c0(game, 0xDF) && !host.unit_0047b850(at<void*>(s, 0xB8)) &&
        (host.plane_hud_cockpit_view_flag_00604f30(s) != 0 || host.global_00f88a32() != 0)) {
        at<std::int32_t>(s, 0xFC) = 1 - at<std::int32_t>(s, 0xFC);
        host.plane_hud_apply_view_mode_00608280(s);
    }
    host.plane_hud_update_view_fov_006068a0(s);
    if (at<void*>(s, 0xB8) == nullptr) {
        at<std::uint8_t>(s, 4) = 0;
        return;
    }

    if (at<std::uint8_t>(at<void*>(s, 0xB8), 0x5D) == 0) {
        at<float>(s, 0x7C) = at<float>(s, 0x7C) - dt;
        float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};  // [ESP0+64h]
        // 0060ADE6..0060AF26: effect +6Ch (definition +68h) while +9F8h.
        if (host.unit_00604a50(at<void*>(s, 0xB8))) {
            void* const unit = at<void*>(s, 0xB8);
            if (at<std::uint8_t>(unit, 0x9F8) != 0) {
                if (at<void*>(s, 0x6C) == nullptr) {
                    create_effect(0x6C, 0x68, at<void*>(unit, 0x4A4), identity);
                }
            } else {
                stop_effect(0x6C);
            }
        }
        // 0060AF2B..0060B092: effect +70h (definition +78h when [[unit+538h]+144h],
        // else +74h) while +9F8h.
        {
            void* const unit = at<void*>(s, 0xB8);
            if (at<std::uint8_t>(unit, 0x9F8) != 0) {
                if (at<void*>(s, 0x70) == nullptr) {
                    const std::size_t definition =
                        at<std::uint8_t>(at<void*>(unit, 0x538), 0x144) != 0 ? 0x78 : 0x74;
                    create_effect(0x70, definition, at<void*>(unit, 0x4A4), identity);
                }
            } else {
                stop_effect(0x70);
            }
        }
        // 0060B097..0060B10C: two draws of the shared stream (00BD2F10, ECX = 1)
        // jitter the camera +94h's +3B4h/+3B8h by +-0.01 while +9F8h.
        if (at<std::uint8_t>(at<void*>(s, 0xB8), 0x9F8) != 0 && at<void*>(s, 0x94) != nullptr) {
            void* camera = at<void*>(s, 0x94);
            const float r0 = host.random_uniform_float_range_00bd2f10(1, -0.01f, 0.01f);
            at<float>(camera, 0x3B4) = r0 + at<float>(camera, 0x3B4);
            camera = at<void*>(s, 0x94);
            const float r1 = host.random_uniform_float_range_00bd2f10(1, -0.01f, 0.01f);
            at<float>(camera, 0x3B8) = r1 + at<float>(camera, 0x3B8);
        }

        // 0060B112..0060B252: sweep probe from 50 to 5000 ahead along the frame's
        // row 2 (one segment in the stack contact list, count +C0h = 0 first);
        // a hit calls unit vtable +38h (ST0 dropped).
        alignas(4) std::uint8_t list[0xC4];  // [ESP0+104h]
        at<std::uint32_t>(list, 0xC0) = 0;
        float far_point[3];  // [ESP0+20h]
        {
            const float* const w = host.entity_get_world_matrix_0042d7e0(at<void*>(s, 0xB8));
            const float t0 = pc24(static_cast<double>(w[8]) * 5000.0);
            const float t1 = pc24(static_cast<double>(w[9]) * 5000.0);
            const float t2 = pc24(5000.0 * static_cast<double>(w[10]));
            const float* const p = host.entity_world_position_00427eb0(at<void*>(s, 0xB8));
            far_point[0] = t0 + p[0];
            far_point[1] = p[1] + t1;
            far_point[2] = p[2] + t2;
        }
        float near_point[3];  // [ESP0+40h]
        {
            const float* const w = host.entity_get_world_matrix_0042d7e0(at<void*>(s, 0xB8));
            const float t0 = pc24(static_cast<double>(w[8]) * 50.0);
            const float t1 = pc24(static_cast<double>(w[9]) * 50.0);
            const float t2 = pc24(50.0 * static_cast<double>(w[10]));
            const float* const p = host.entity_world_position_00427eb0(at<void*>(s, 0xB8));
            near_point[0] = t0 + p[0];
            near_point[1] = p[1] + t1;
            near_point[2] = p[2] + t2;
        }
        host.contact_point_list_append_0042b570(list, near_point, far_point);
        alignas(4) std::uint8_t hit[0x54];  // [ESP0+B0h]
        host.hit_record_reset_00470470(hit);
        void* const index = host.spatial_index_singleton_0042e630();
        if (host.spatial_index_sweep_segments_0098b370(index, list, 0, hit)) {
            host.unit_vcall_38(at<void*>(s, 0xB8));
        }
        // 0060B254..0060B43B: speed above 200 (|v| * 2.412) keeps the wind effect
        // +64h (definition +60h, 40 above the unit), aimed along -v.
        float velocity_buffer[3];  // [ESP0+40h]
        const float* const v = host.unit_vcall_34_velocity(at<void*>(s, 0xB8), velocity_buffer);
        const float speed = pc24(static_cast<double>(host.vector3_length_00419440(v)) * 2.412);
        if (speed > 200.0f) {
            if (at<void*>(s, 0x64) == nullptr) {
                const float offset[3] = {0.0f, 0.0f, 40.0f};  // [ESP0+40h]
                float translation[16];                        // [ESP0+64h]
                float* const m = host.transform_init_translation_0047c240(translation, offset);
                create_effect(0x64, 0x60, at<void*>(at<void*>(s, 0xB8), 0x4A4), m);
            }
            if (at<void*>(s, 0x64) != nullptr) {
                float vb[3];  // [ESP0+A4h]
                const float* const u = host.unit_vcall_34_velocity(at<void*>(s, 0xB8), vb);
                void* const effect = at<void*>(s, 0x64);
                float back[3] = {neg_zero - u[0], neg_zero - u[1], neg_zero - u[2]};  // [ESP0+40h]
                float dir[3];                                                         // [ESP0+20h]
                host.vector3_normalize_00419510(dir, back);
                at<float>(effect, 0x5C) = dir[0];
                at<float>(effect, 0x60) = dir[1];
                at<float>(effect, 0x64) = dir[2];
                at<std::int32_t>(effect, 0x28) = 0;
            }
        } else {
            stop_effect(0x64);
        }
        host.hit_record_destroy_004704b0(hit);
    } else {
        // 0060B457..0060B479
        stop_effect(0x64);
    }

    // 0060B47E..0060B64C: free look on action A0h (+9Ch latched, unless +BDh).
    const bool look_start = at<std::uint8_t>(s, 0x9C) == 0 && at<std::uint8_t>(s, 0xBD) == 0 &&
                            host.input_action_is_held_004c5090(game, 0xA0);
    const bool look_end = at<std::uint8_t>(s, 0x9C) != 0 && !host.input_action_is_held_004c5090(game, 0xA0);
    if (look_start) {
        at<std::uint8_t>(s, 0x9C) = 1;
        at<float>(s, 0xA0) = 0.0f;
        at<float>(s, 0xA4) = 0.0f;
    } else if (look_end) {
        if (at<void*>(s, 0x98) != nullptr) {
            host.cockpit_camera_0042aa00(at<void*>(s, 0x98), 0.0f, 0.0f, 0);
        }
        void* const camera = at<void*>(s, 0x94);
        if (camera != nullptr) {
            at<float>(camera, 0x3E8) = 0.0f;
            at<float>(camera, 0x3E4) = 0.0f;
        }
        at<std::uint8_t>(s, 0x9C) = 0;
    }

    if (at<std::uint8_t>(s, 0x9C) != 0) {
        // 0060B500..0060B52B: the commanded target object (may be null).
        void* const command = at<void*>(at<void*>(s, 0xB8), 0x9D4);
        void* const descriptor_owner = host.command_vcall_114(command);
        void* const descriptor = host.entity_command_active_target_descriptor_0071eb60(descriptor_owner);
        void* target = host.command_target_resolve_00521ea0(descriptor);

        // Aim at an entity: pitch (EDX output) and yaw (stack output) of the
        // floor-normalised direction in the unit frame.
        const auto aim_at = [&](void* entity, float* out_pitch, float* out_yaw) {
            float d[3];  // [ESP0+40h]
            const float* const own = host.entity_world_position_00427eb0(at<void*>(s, 0xB8));
            const float* const other = host.entity_world_position_00427eb0(entity);
            host.vector3_subtract_to_00427c90(other, d, own);
            host.geometry_normalize_with_floor_0042b260(d);
            const float* const inverse = host.entity_pose_derived_affine_inverse_00414e10(at<void*>(s, 0xB8));
            float local[3];  // [ESP0+20h]
            host.vector3_transform_direction_0042d0d0(local, d, inverse, 0);
            host.direction_to_pitch_yaw_00521370(local, out_pitch, out_yaw);
        };

        bool cockpit_path = at<void*>(s, 0x94) == nullptr;
        if (!cockpit_path) {
            void* const unit = at<void*>(s, 0xB8);
            bool tracked = false;
            if (at<std::uint8_t>(s, 0xBD) == 0 && target != nullptr && target != unit &&
                target != at<void*>(unit, 0x9D4)) {
                // 0060B562..0060B5FC: a ship (kind 5) or an aircraft carrier group (18h).
                if (host.entity_vcall_5c_is_kind(target, 5) || host.entity_vcall_5c_is_kind(target, 0x18)) {
                    if (host.entity_vcall_5c_is_kind(target, 0x18)) {
                        target = at<void*>(target, 0x3D0);
                    }
                    float pitch;  // [ESP0+10h]
                    float yaw;    // [ESP0+14h]
                    aim_at(target, &pitch, &yaw);
                    void* const camera = at<void*>(s, 0x94);
                    at<float>(camera, 0x3E8) = neg_zero - pitch;
                    at<float>(camera, 0x3E4) = yaw;
                    tracked = true;
                }
            }
            if (!tracked) {
                if (at<void*>(s, 0x94) == nullptr || at<std::uint8_t>(s, 0xBD) != 0) {
                    cockpit_path = true;
                } else {
                    // 0060B66B..0060B7D2: chase camera free look from the hold
                    // times of B0h/B1h (accumulated into +A0h/+A4h, clamped to
                    // +-pi / +-pi/2) and A1h/A2h.
                    float* const yaw_acc = &at<float>(s, 0xA0);
                    float* const pitch_acc = &at<float>(s, 0xA4);
                    *yaw_acc = host.input_action_hold_time_004c5070(game, 0xB0) + *yaw_acc;
                    const double pitch_before = static_cast<double>(*pitch_acc);
                    const float b1 = host.input_action_hold_time_004c5070(game, 0xB1);
                    *pitch_acc = pc24(pitch_before - static_cast<double>(b1));
                    float hi = 3.14159274f;    // 00D7A264
                    float lo = -3.14159274f;   // 00CE684C
                    host.math_clamp_in_place_00415690(yaw_acc, &lo, &hi);
                    hi = 1.57079637f;   // 00CE3C64
                    lo = -1.57079637f;  // 00CE3CCC
                    host.math_clamp_in_place_00415690(pitch_acc, &lo, &hi);
                    const float a1 = host.input_action_hold_time_004c5070(game, 0xA1);
                    const float yaw =
                        pc24(static_cast<double>(pc24(static_cast<double>(a1) * 3.1415927410125732)) + *yaw_acc);
                    const double pitch_now = static_cast<double>(*pitch_acc);
                    const float a2 = host.input_action_hold_time_004c5070(game, 0xA2);
                    float pitch = pc24(pitch_now -
                                       static_cast<double>(pc24(static_cast<double>(a2) * 1.5707963705062866)));
                    if (host.entity_ancestor_00923810(at<void*>(s, 0xB8), 1) != nullptr) {
                        float zero = 0.0f;
                        pitch = max_native_float_by_ref_00415550(&pitch, &zero);
                    }
                    float altitude = host.entity_world_position_00427eb0(at<void*>(s, 0xB8))[1];
                    // FLD 50.0; FCOMI; JBE: below 50 the pitch floor rises to -pi/2*alt/50.
                    if (50.0 > static_cast<double>(altitude)) {
                        altitude = pc24(static_cast<double>(pc24(static_cast<double>(altitude) / 50.0)) *
                                        -1.5707963705062866);
                        pitch = max_native_float_by_ref_00415550(&pitch, &altitude);
                    }
                    void* const camera = at<void*>(s, 0x94);
                    at<float>(camera, 0x3E8) = pitch;
                    at<float>(camera, 0x3E4) = yaw;
                }
            }
        }
        if (cockpit_path && at<void*>(s, 0x98) != nullptr && host.unit_vcall_1c0(at<void*>(s, 0xB8)) != 0) {
            // 0060B7FC..0060BA0B: cockpit free look, limits from game tuning
            // +A0h (yaw), +A8h (pitch up) and +A4h (pitch down).
            const float yaw_limit = at<float>(host.game_tuning_get_0042e740(), 0xA0);   // [ESP0+2Ch]
            float pitch_up = at<float>(host.game_tuning_get_0042e740(), 0xA8);          // [ESP0+14h]
            const float pitch_down = at<float>(host.game_tuning_get_0042e740(), 0xA4);  // [ESP0+10h]
            float out_a;  // [ESP0+1Ch]
            float out_b;  // [ESP0+18h]
            void* const unit = at<void*>(s, 0xB8);
            if (target != nullptr && target != unit && target != at<void*>(unit, 0x9D4)) {
                float pitch;  // [ESP0+18h]
                float yaw;    // [ESP0+1Ch]
                aim_at(target, &pitch, &yaw);
                out_b = neg_zero - pitch;
                out_a = yaw;
            } else {
                float* const yaw_acc = &at<float>(s, 0xA0);
                float* const pitch_acc = &at<float>(s, 0xA4);
                const float b1 = host.input_action_hold_time_004c5070(game, 0xB1);
                const float b0 = host.input_action_hold_time_004c5070(game, 0xB0);
                *yaw_acc = *yaw_acc + b0;
                *pitch_acc = *pitch_acc - b1;
                float lo = neg_zero - yaw_limit;
                float yaw_hi = yaw_limit;
                host.math_clamp_in_place_00415690(yaw_acc, &lo, &yaw_hi);
                float pitch_lo = neg_zero - pitch_down;  // [ESP0+1Ch]; also stored at [ESP0+20h]
                host.math_clamp_in_place_00415690(pitch_acc, &pitch_lo, &pitch_up);
                const float a1 = host.input_action_hold_time_004c5070(game, 0xA1);
                const float a2 = host.input_action_hold_time_004c5070(game, 0xA2);
                float neg_down = pitch_lo;  // [ESP0+20h]
                const float scaled = a1 * yaw_limit;
                out_a = scaled + *yaw_acc;
                const float m = min_native_float_by_ref_00415510(&pitch_up, &neg_down);
                const float pm = m * a2;
                out_b = pm + *pitch_acc;
            }
            // 0060B9A5..0060BA0B
            float yaw_limit_copy = yaw_limit;
            float lo = neg_zero - yaw_limit;
            host.math_clamp_in_place_00415690(&out_a, &lo, &yaw_limit_copy);
            lo = neg_zero - pitch_down;
            host.math_clamp_in_place_00415690(&out_b, &lo, &pitch_up);
            host.cockpit_camera_0042aa00(at<void*>(s, 0x98), out_a, out_b, 0);
        }
    }

    // 0060BA10..0060BA32: the pilot command block [ESP0+4Ch]: +0 rudder, +4
    // elevator, +8 aileron, +0Ch throttle (sqrt), +10h brake, +14h bytes
    // (+14h fire held AAh, +16h bomb byte).
    alignas(4) std::uint8_t block[0x18];
    at<std::int32_t>(block, 0x0C) = 0;
    at<std::int32_t>(block, 0x00) = 0;
    at<std::int32_t>(block, 0x04) = 0;
    at<std::int32_t>(block, 0x08) = 0;
    at<std::int32_t>(block, 0x10) = 0;
    at<std::int32_t>(block, 0x14) = 0;
    at<float>(block, 0x0C) = 1.0f;

    // 0060BA38..0060BACC: raw stick axes from the input manager.
    float axis_x = at<float>(at<void*>(host.input_manager_singleton_004bec00(), 4), 0x20C4);  // [ESP0+10h]
    float axis_y = at<float>(at<void*>(host.input_manager_singleton_004bec00(), 4), 0x20F4);  // [ESP0+14h]
    const float axis_bx = at<float>(at<void*>(host.input_manager_singleton_004bec00(), 4), 0x1F14);
    const float axis_by = at<float>(at<void*>(host.input_manager_singleton_004bec00(), 4), 0x1F44);
    if (at<std::uint8_t>(s, 0x9C) != 0 && host.global_00e188d8() != nullptr &&
        host.party_00927880(host.global_00e188d8()) != nullptr &&
        host.party_00927880(host.global_00e188d8()) == host.global_00e188d8()) {
        axis_y = 0.0f;
        axis_x = 0.0f;
    }
    const float filtered_x = host.plane_hud_axis_filter_006062b0(at<void*>(s, 0xE8), axis_x, axis_bx, dt);
    const float filtered_y = host.plane_hud_axis_filter_006062b0(at<void*>(s, 0xEC), axis_y, axis_by, dt);
    host.unit_can_drop_ordnance_007b9140(at<void*>(s, 0xB8), 0);  // AL unused
    const float aileron_scale = host.plane_hud_cockpit_view_flag_00604f30(s) == 0
                                    ? 1.0f
                                    : at<float>(host.game_tuning_get_0042e740(), 0x680);
    const float elevator_scale = host.plane_hud_cockpit_view_flag_00604f30(s) == 0
                                     ? 1.0f
                                     : at<float>(host.game_tuning_get_0042e740(), 0x67C);
    at<float>(block, 0x08) = aileron_scale * filtered_x;
    at<float>(block, 0x04) = elevator_scale * filtered_y;

    // 0060BB9B..0060BBD5: latch 00E19694 while the priority lists reach level
    // 2; released once action A3h's hold time is exactly 0.
    std::uint8_t* const throttle_latch = host.global_00e19694();
    if (host.priority_lists_top_level_004f7740() > 1) {
        *throttle_latch = 1;
    } else if (*throttle_latch != 0) {
        if (host.input_action_hold_time_004c5070(game, 0xA3) == 0.0f) {
            *throttle_latch = 0;
        }
    }
    // 0060BBDC..0060BC6C: throttle source flags.
    const bool forces_unit_scale = host.input_action_any_binding_forces_unit_scale_00a92050(
        bytes(at<void*>(host.input_manager_singleton_004bec00(), 4)) + 0x1E90);  // [ESP0+3Bh]
    const bool has_device = host.input_action_has_device_query_00a92090(
        bytes(at<void*>(host.input_manager_singleton_004bec00(), 4)) + 0x1E90);
    bool absolute_throttle;  // [ESP0+33h]
    std::uint8_t enabled;    // low byte of [ESP0+10h]
    if (!has_device && !forces_unit_scale) {
        absolute_throttle = false;
        enabled = host.input_action_is_enabled_00606250(game, 0xA3) ? 1 : 0;
    } else {
        absolute_throttle = true;
        enabled = forces_unit_scale ? 1 : (host.input_action_is_enabled_00606250(game, 0xA3) ? 1 : 0);
    }
    std::uint8_t use_throttle = *throttle_latch != 0 ? 0 : enabled;  // BL
    const float throttle = *throttle_latch != 0 ? 0.0f : host.input_action_hold_time_004c5070(game, 0xA3);
    // 0060BC6C..0060BCC7: a moved throttle ends the +10Ch hold.
    if (at<std::uint8_t>(s, 0x10C) != 0 && forces_unit_scale) {
        if (throttle == at<float>(s, 0x108)) {
            use_throttle = 0;
        } else {
            at<std::uint8_t>(s, 0x10C) = 0;
            if (host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 0) &&
                !host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 1)) {
                host.unit_0077c470(at<void*>(s, 0xB8), 2, 1);
            }
        }
    }
    // 0060BCCC..0060BD0E: squared current throttle +9F0h; the incremental input.
    const float current = at<float>(at<void*>(s, 0xB8), 0x9F0);
    float throttle_sq = current * current;  // [ESP0+18h]
    float delta = at<std::uint8_t>(host.global_00e188a8(), 0x634) != 0 ? 0.0f : neg_zero - throttle;  // [ESP0+1Ch]

    bool run_rudder = at<std::uint8_t>(s, 0x9C) == 0;
    if (!run_rudder) {
        // 0060BD16..0060BD76: during free look only a ship or carrier-group party
        // object other than the party itself keeps the rudder.
        void* const party = host.global_00e188d8();
        if (party != nullptr && host.party_00927880(host.global_00e188d8()) != nullptr &&
            host.party_00927880(host.global_00e188d8()) != host.global_00e188d8()) {
            if (host.entity_vcall_5c_is_kind(host.party_00927880(host.global_00e188d8()), 5) ||
                host.entity_vcall_5c_is_kind(host.party_00927880(host.global_00e188d8()), 0x18)) {
                run_rudder = true;
            }
        }
    }
    bool doubled_brake = false;  // 0060BF28 path
    if (run_rudder) {
        // 0060BD7C..0060BE76: rudder A4h scaled by the elevator scale, cut by the
        // HUD state; a digital stick keeps only its larger axis.
        const float rudder_raw = host.input_action_hold_time_004c5070(game, 0xA4) * elevator_scale;
        void* const gui = host.global_00e198c4();
        bool cut = false;
        if (at<std::uint8_t>(at<void*>(gui, 0xC0), 4) != 0 || host.hud_006529e0(at<void*>(gui, 0xBC))) {
            void* const b8 = host.global_00e198b8();
            if (b8 != nullptr && at<void*>(b8, 0x40) != nullptr &&
                host.bit_flag_set_test_00608c00(at<void*>(b8, 0x40), 0)) {
                cut = true;
            }
        }
        float rudder = cut ? 0.0f : rudder_raw;
        if (!absolute_throttle && 1.0f > abs_bits(rudder) && 1.0f > abs_bits(delta)) {
            // FLD |delta|; FLD |rudder|; FCOMIP; JBE: ties and NaN zero the rudder.
            if (abs_bits(rudder) > abs_bits(delta)) {
                delta = 0.0f;
            } else {
                rudder = 0.0f;
            }
        }
        at<float>(block, 0x00) = rudder;
        if (!absolute_throttle) {
            const float step = delta * dt;
            throttle_sq = step + throttle_sq;
        } else if (use_throttle != 0) {
            if (forces_unit_scale) {
                throttle_sq = pc24(static_cast<double>(pc24(static_cast<double>(throttle) * -0.5)) + 0.5);
            } else {
                const float lever = neg_zero - throttle;
                if (lever > -0.5f) {
                    throttle_sq = host.math_interpolate_clamped_00419010(-0.5f, 0.0f, 1.0f, 1.0f, lever);
                } else {
                    doubled_brake = true;
                }
            }
        }
    }
    // 0060BEF0..0060BF5E: a negative throttle target brakes (+10h), otherwise
    // the target is capped at 1.
    if (doubled_brake) {
        throttle_sq = 0.0f;
        if (use_throttle != 0) {
            const float twice = throttle + throttle;
            at<float>(block, 0x10) = abs_bits(twice);
        }
    } else if (0.0f > throttle_sq) {
        throttle_sq = 0.0f;
        if (!absolute_throttle) {
            at<float>(block, 0x10) = abs_bits(throttle);
        } else if (use_throttle != 0) {
            const float twice = throttle + throttle;
            at<float>(block, 0x10) = abs_bits(twice);
        }
    } else {
        at<float>(block, 0x10) = 0.0f;
        if (throttle_sq > 1.0f) {
            throttle_sq = 1.0f;
        }
    }
    // 0060BF64..0060BF91: throttle = sqrt (00BF7030), fire held AAh.
    at<float>(block, 0x0C) = static_cast<float>(host.crt_sqrt_00bf7030(static_cast<double>(throttle_sq)));
    at<std::uint8_t>(block, 0x14) = host.input_action_is_held_004c5090(game, 0xAA) ? 1 : 0;

    // 0060BF95..0060C09A: any flight action pressed hands the role back.
    if (host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 0) &&
        !host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 1)) {
        static const std::int32_t actions[] = {0xAE, 0xAF, 0xA5, 0xA6, 0xA3, 0xAA, 0xA4, 0xA9, 0xA8, 0xA7};
        bool pressed = false;
        for (const std::int32_t action : actions) {
            if (host.input_action_was_pressed_004c50d0(game, action)) {
                pressed = true;
                break;
            }
        }
        if (pressed) {
            host.unit_0077c470(at<void*>(s, 0xB8), 2, 1);
            at<std::uint8_t>(s, 0x10C) = 0;
        }
    }
    // 0060C0A1..0060C0C7: bomb fire for the local pilot.
    if (host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 1)) {
        host.plane_hud_update_bomb_fire_006082d0(s, block);
    } else {
        at<std::uint8_t>(s, 0xBC) = 0;
        at<std::uint8_t>(block, 0x16) = 0;
    }
    // 0060C0CC..0060C22B: clamp and send the pilot command block. Below 5
    // altitude the elevator floor rises from -1 (00419010 over +C64h).
    if (host.unit_is_local_player_role_00927f30(at<void*>(s, 0xB8), 1)) {
        float elevator_low = -1.0f;  // [ESP0+10h]
        if (5.0f > host.entity_world_position_00427eb0(at<void*>(s, 0xB8))[1]) {
            void* const tuning = host.game_tuning_get_0042e740();
            const float x = at<float>(at<void*>(s, 0xB8), 0xC64);
            const float y1 = neg_zero - at<float>(tuning, 0x2B8);
            elevator_low = host.math_interpolate_clamped_00419010(0.0f, -1.0f, y1, 0.0f, x);
        }
        float hi = 1.0f;
        float lo = 0.0f;
        host.math_clamp_in_place_00415690(&at<float>(block, 0x0C), &lo, &hi);
        hi = 1.0f;
        lo = -1.0f;
        host.math_clamp_in_place_00415690(&at<float>(block, 0x00), &lo, &hi);
        hi = 1.0f;
        lo = -1.0f;
        host.math_clamp_in_place_00415690(&at<float>(block, 0x08), &lo, &hi);
        hi = 1.0f;
        host.math_clamp_in_place_00415690(&at<float>(block, 0x04), &elevator_low, &hi);
        hi = 1.0f;
        elevator_low = 0.0f;
        host.math_clamp_in_place_00415690(&at<float>(block, 0x10), &elevator_low, &hi);
        host.plane_set_pilot_command_block_007b8c90(at<void*>(s, 0xB8), block);
    }

    // 0060C230..0060C338: bomb-release timer +CCh. On expiry with a target +E4h:
    // a bomber (kind 10h) with a loaded bomb class and more than 0.7 rounds
    // re-arms at 0.01 observing the class (+D0h); otherwise the target's
    // +108h object, when its +38h holds and neither HUD panel is open, pushes
    // interface request 26h, else the timer restarts from settings +460h.
    {
        float timer = at<float>(s, 0xCC);
        if (timer > 0.0f) {
            timer = timer - dt;
            at<float>(s, 0xCC) = timer;
            if (timer <= 0.0f && at<void*>(s, 0xE4) != nullptr) {
                bool rearmed = false;
                if (host.entity_vcall_5c_is_kind(at<void*>(s, 0xB8), 0x10)) {
                    void* const bomb_class = host.weapon_first_bomb_class_007b9510(at<void*>(s, 0xB8));
                    if (bomb_class != nullptr && at<std::uint8_t>(bomb_class, 0x5C) != 0 &&
                        host.weapon_total_bomb_rounds_007bc8c0(at<void*>(s, 0xB8)) > 0.699999988f) {
                        host.observer_ref_set_0042eca0(bytes(s) + 0xD0, bomb_class);
                        at<float>(s, 0xCC) = 0.01f;
                        rearmed = true;
                    }
                }
                if (!rearmed) {
                    void* const object = host.object_vcall_108(at<void*>(s, 0xE4));
                    if (object != nullptr && host.object_vcall_38_bool(object) &&
                        at<std::uint8_t>(at<void*>(host.global_00e198c4(), 0xBC), 5) == 0 &&
                        at<std::uint8_t>(at<void*>(host.global_00e198c4(), 0xC0), 5) == 0) {
                        host.frontend_push_interface_request_004cc460(host.global_00e198c4(), 0x26,
                                                                      at<void*>(s, 0xE4));
                    } else {
                        at<float>(s, 0xCC) = at<float>(host.game_settings_get_00424c40(), 0x460);
                    }
                }
            }
        }
    }
    host.hud_plane_update_instruments_00609bd0(at<void*>(host.global_00e198c4(), 0x68), dt,
                                               at<std::uint8_t>(s, 0xBD), flag_c24);

    // 0060C364..0060C58B: zoom FOV. The unit's +204h value goes into the
    // 16-slot ring +1Ch (index +5Ch); with +9F8h the ring mean, clamped to
    // tuning +B8h..+BCh, picks the zoom through 00419010 (+B8h,+C0h,+BCh,+C4h),
    // times the camera FOV; +84h steps toward it, +80h follows by dt, and the
    // chase camera +94h gets tan(+80h/2)/tan(fov/2) at +384h.
    {
        void* const unit = at<void*>(s, 0xB8);
        void* const g = host.global_00e188a8();
        if (unit == nullptr || g == nullptr || at<void*>(g, 0x19FC) == nullptr || !(dt > 0.0f)) return;
        const float fov = at<float>(at<void*>(g, 0x19FC), 0x1C4);  // [ESP0+10h]
        const float value = host.unit_vcall_204(unit);
        at<float>(s, 0x1C + at<std::int32_t>(s, 0x5C) * 4) = value;
        at<std::uint32_t>(s, 0x5C) = (at<std::uint32_t>(s, 0x5C) + 1u) & 0xFu;
        float mean = 0.0f;  // [ESP0+18h]
        if (at<std::uint8_t>(at<void*>(s, 0xB8), 0x9F8) != 0) {
            for (int i = 0; i < 16; ++i) {
                mean = at<float>(s, 0x1C + static_cast<std::size_t>(i) * 4) + mean;
            }
            mean = pc24(static_cast<double>(mean) * 0.0625);
        }
        void* const t_hi = host.game_tuning_get_0042e740();
        void* const t_lo = host.game_tuning_get_0042e740();
        mean = host.float_clamp_by_pointer_004155b0(&mean, &at<float>(t_lo, 0xB8), &at<float>(t_hi, 0xBC));
        void* const ta = host.game_tuning_get_0042e740();
        void* const tb = host.game_tuning_get_0042e740();
        void* const tc = host.game_tuning_get_0042e740();
        // (mean - lo) * pi / (hi - lo), then FCOS into [ESP0+3Ch]; that value is
        // overwritten below before any read (dead in the image).
        const float span_num = pc24(static_cast<double>(mean - at<float>(ta, 0xB8)) * 3.1415927410125732);
        const float span = at<float>(tb, 0xBC) - at<float>(tc, 0xB8);
        const float phase = span_num / span;
        static_cast<void>(phase);
        void* const t1 = host.game_tuning_get_0042e740();
        void* const t2 = host.game_tuning_get_0042e740();
        void* const t3 = host.game_tuning_get_0042e740();
        void* const t4 = host.game_tuning_get_0042e740();
        const float zoom = host.math_interpolate_clamped_00419010(at<float>(t4, 0xB8), at<float>(t3, 0xC0),
                                                                  at<float>(t2, 0xBC), at<float>(t1, 0xC4), mean);
        const float wanted = zoom * fov;
        host.math_step_towards_0042ac60(&at<float>(s, 0x84), wanted, dt);
        const float current_fov = at<float>(s, 0x80);
        const float gap = at<float>(s, 0x84) - current_fov;
        const float step = gap * dt;
        const float next = current_fov + step;
        at<float>(s, 0x80) = next;
        host.camera_set_field_of_view_00b6fbb0(at<void*>(host.global_00e188a8(), 0x19FC), next);
        void* const chase = at<void*>(s, 0x94);
        if (chase != nullptr) {
            const float half = pc24(static_cast<double>(at<float>(s, 0x80)) * 0.5);
            const float tan_half = static_cast<float>(host.crt_citan_00bfa020(static_cast<double>(half)));
            const float half_fov = pc24(static_cast<double>(fov) * 0.5);
            const float tan_fov = static_cast<float>(host.crt_citan_00bfa020(static_cast<double>(half_fov)));
            at<float>(chase, 0x384) = tan_half / tan_fov;
        }
    }
}

// ---------------------------------------------------------------------------
// 00822C20 BSP_UnitInstance_SEntityInit
// ---------------------------------------------------------------------------
//
// __thiscall(ECX = unit), RET. EBP frame with ESP aligned to 8, SEH (handler
// 00C917F1, state at [ESP1+134h]: 0..12h around the temporary references),
// unwinding not modelled. Retained references are inlined in the image: the
// count is at +4h, incremented / decremented through the imports
// InterlockedIncrement (00CE221C) and InterlockedDecrement (00CE2220), and a
// count reaching 0 calls the object's vtable slot 0. Constants from the PE:
// 00CE3800 0.5f, 00D7A24C 1.0f, 00D7A280 double 0.5, 00CE3DC0 double 10.0,
// 00CF0AB8 double 9.0, 00D7A2F0 0.1f, 00CE3914 9.0f, 00CE38B8 10.0f. Strings
// from the PE: "Sound" 00CEA3FC, ShipYardLaunch 00D0659C, StartSpeed 00CFCCFC,
// CamoColor 00D09980, CamoColorGun 00D09970, "R"/"G"/"B"/"A" 00CE60EC/E8/E4/
// 00CE608C, "waterclip" 00D09964, Navigator 00D09958, ArtilleryDirector
// 00D09944, TorpedoDirector 00D09934, AADirector 00D09928, DCDirector
// 00D0991C, FireStance 00CFC850, PlayerCommandsEnabled 00D08AF8,
// TorpedoEvasion 00D0990C, ShipCollAvoid 00D098FC, LandCollAvoid 00D098EC,
// NavigatorAllowMaxDepth 00D098D4. The "-(p != 0) & 0E17BF8h / 0E186ECh"
// tests are p != 0.
void cc13_bigB1_unit_sentity_init_00822c20(void* unit, Cc13BigB1Host& host) {
    void* const u = unit;

    // Inlined retained-reference helpers.
    const auto add_ref = [&](void* object) {
        host.interlocked_increment_00ce221c(&at<std::int32_t>(object, 4));
    };
    const auto release = [&](void* object) {
        if (object != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    const auto assign = [&](std::size_t slot, void* value) {
        void* const old = at<void*>(u, slot);
        if (old != value) {
            at<void*>(u, slot) = value;
            if (value != nullptr) add_ref(value);
            release(old);
        }
    };
    // The definition copied into the by-value argument slot.
    const auto copy_arg = [&](void* definition) -> void* {
        void* arg = nullptr;
        if (definition != nullptr) {
            arg = definition;
            add_ref(definition);
        }
        return arg;
    };
    const auto make_identity = [](float* m) {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = 1.0f;
        m[5] = 1.0f;
        m[10] = 1.0f;
        m[15] = 1.0f;
    };
    // 008687C0 under the unit's node +4A4h into a stack reference, assigned to
    // the unit slot, then released.
    const auto create_parented = [&](std::size_t slot, void* definition, const float* matrix) {
        void* const parent = at<void*>(u, 0x4A4);
        void* const arg = copy_arg(definition);
        void* created = nullptr;
        void* const result =
            host.point_effect_create_with_parent_matrix_008687c0(&created, parent, arg, matrix, 1, 0, 0);
        assign(slot, at<void*>(result, 0));
        release(created);
    };
    // 00868420 in the effect system [[00E188A8]+19ECh].
    const auto create_in_system = [&](std::size_t slot, void* definition, const float* matrix) {
        void* const system = at<void*>(host.global_00e188a8(), 0x19EC);
        void* const arg = copy_arg(definition);
        void* created = nullptr;
        void* const result = host.point_effect_create_from_matrix_00868420(&created, system, arg, matrix, 0, 0);
        assign(slot, at<void*>(result, 0));
        release(created);
    };
    // The "Sound" child of an effect into a second slot (006FBEB0), the name
    // string returned to the sized pool.
    const auto bind_sound = [&](std::size_t effect_slot, std::size_t sound_slot) {
        if (at<void*>(u, effect_slot) == nullptr) return;
        alignas(4) std::uint8_t name[8];  // NativeString {+0 length, +4 data}
        host.native_string_assign_0041e870(name, "Sound");
        void* child = nullptr;
        void* const result = host.point_effect_find_or_create_named_child_00866cd0(at<void*>(u, effect_slot),
                                                                                   &child, name);
        host.retained_reference_assign_from_slot_006fbeb0(bytes(u) + sound_slot, result);
        if (child != nullptr) {
            release(child);
            child = nullptr;
        }
        if (at<void*>(name, 4) != nullptr) {
            const std::uint32_t size = at<std::uint32_t>(name, 0) + 1;
            void* const pool = host.sized_pool_get_00419cc0();
            host.sized_pool_return_block_00bd1510(pool, at<void*>(name, 4), size, 1);
        }
    };
    // The cached world matrix +CCh (valid byte +C8h), refreshed from the local
    // +74h and the parent +3Ch (inlined in the image at each use).
    const auto refresh_world = [&] {
        if (at<std::uint8_t>(u, 0xC8) != 0) return;
        if (at<void*>(u, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(at<void*>(u, 0x3C));
        float scratch[16];
        const float* source = &at<float>(u, 0x74);
        if (at<void*>(u, 0x3C) != nullptr) {
            source = host.matrix_multiply_00413920(&at<float>(u, 0x74), scratch,
                                                   &at<float>(at<void*>(u, 0x3C), 0xCC));
        }
        host.matrix_copy_4x4_004134f0(&at<float>(u, 0xCC), source);
        at<std::uint8_t>(u, 0xC8) = 1;
        at<std::uint8_t>(u, 0x10C) = 0;
    };
    // [00E0C978] and [[00F88C30]+B8h]: the gameplay modifier product (kind 4).
    const auto modifier = [&]() -> float {
        if (host.global_00e0c978() != 0 && at<std::int32_t>(host.global_00f88c30(), 0xB8) != 0) {
            return static_cast<float>(
                host.gameplay_modifiers_product_for_unit_008e6430(host.global_00f88c30(), 4, u));
        }
        return 1.0f;
    };
    // A property-bag number: type +4h 0 = int (CVTSI2SS), else the float bits.
    const auto bag_number = [&](void* record) -> float {
        if (at<std::int32_t>(record, 4) == 0) return static_cast<float>(at<std::int32_t>(record, 0xC));
        return at<float>(record, 0xC);
    };
    const auto bag = [&]() -> void* { return at<void*>(at<void*>(u, 0xC0), 8); };
    // A boolean key that defaults to true: absent or nonzero -> 1.
    const auto bag_flag = [&](const char* key) -> std::uint8_t {
        void* const record = host.scene_property_bag_find_008f2260(bag(), key);
        return (record == nullptr || at<std::uint8_t>(record, 0xC) != 0) ? 1 : 0;
    };

    // 00822C49..00822CDB: class speed, the colour defaults, navigator reset.
    at<float>(u, 0xBC4) = 0.0f;
    at<float>(u, 0x9C0) = at<float>(at<void*>(u, 0x538), 0x500);
    at<float>(u, 0x1160) = 0.5f;
    at<float>(u, 0x1164) = 0.5f;
    at<float>(u, 0x1168) = 0.5f;
    at<std::uint8_t>(u, 0x9C4) = 1;
    at<float>(u, 0x116C) = 1.0f;
    host.unit_reset_navigator_params_00822b70(u, 1);
    host.unit_vcall_10(u);
    host.unit_initialize_scene_bindings_00955420(u);

    // 00822CE0..008234C0: four effects from class +E8h/+108h/+100h/+FCh into
    // +BACh/+BB0h/+BA4h/+BA8h at z = -A0h/2, 0, +A0h/2, -A0h/2, each with its
    // "Sound" child in +BBCh/+BC0h/+BB4h/+BB8h.
    {
        void* cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0xE8) != nullptr) {
            float m[16];
            make_identity(m);
            m[14] = pc24(static_cast<double>(-at<float>(cls, 0xA0)) * 0.5);
            create_parented(0xBAC, at<void*>(cls, 0xE8), m);
            bind_sound(0xBAC, 0xBBC);
        }
        cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0x108) != nullptr) {
            float m[16];
            make_identity(m);
            create_parented(0xBB0, at<void*>(cls, 0x108), m);
            bind_sound(0xBB0, 0xBC0);
        }
        cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0x100) != nullptr) {
            float m[16];
            make_identity(m);
            m[14] = pc24(static_cast<double>(at<float>(cls, 0xA0)) * 0.5);
            create_parented(0xBA4, at<void*>(cls, 0x100), m);
            bind_sound(0xBA4, 0xBB4);
        }
        cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0xFC) != nullptr) {
            float m[16];
            make_identity(m);
            m[14] = pc24(static_cast<double>(-at<float>(cls, 0xA0)) * 0.5);
            create_parented(0xBA8, at<void*>(cls, 0xFC), m);
            bind_sound(0xBA8, 0xBB8);
        }
    }

    // 008234C5..00823529: +1114h resized to the count of 30h-byte records of
    // [+354h]+1Ch (value 0); pose-dependent reset; +104Ch = -1; +1130h = 0;
    // unit vtable +230h.
    {
        void* const holder = at<void*>(u, 0x354);
        std::uint8_t* const first = at<std::uint8_t*>(holder, 0x1C);
        std::uint32_t count = 0;
        if (first != nullptr) {
            const std::int32_t span = static_cast<std::int32_t>(at<std::uint8_t*>(holder, 0x20) - first);
            count = static_cast<std::uint32_t>(span / 0x30);
        }
        host.vector_resize_00822460(bytes(u) + 0x1114, count, 0);
    }
    host.unit_reset_pose_dependent_state_00818ea0(u);
    at<std::int32_t>(u, 0x104C) = -1;
    at<std::int32_t>(u, 0x1130) = 0;
    std::uint8_t yard_launch = 0;  // BL
    host.unit_vcall_230(u);
    at<std::uint8_t>(u, 0x1174) = yard_launch;

    // 0082352B..00823704: the scene record +C0h (kind +4h).
    {
        void* const record = at<void*>(u, 0xC0);
        const std::int32_t kind = at<std::int32_t>(record, 4);
        if (kind == 1) {
            void* found = host.scene_property_bag_find_008f2260(at<void*>(record, 8), "ShipYardLaunch");
            if (found != nullptr) yard_launch = at<std::uint8_t>(found, 0xC);
            found = host.scene_property_bag_find_008f2260(bag(), "StartSpeed");
            if (found != nullptr) {
                // 008235AA..008235F7: throttle = speed / reference speed; the
                // controller gets reference speed * throttle.
                const float speed = bag_number(found);
                const double speed_d = static_cast<double>(speed);
                const float throttle = pc24(speed_d / host.unit_reference_speed_0080fc30(u));
                host.unit_order_ring_set_throttle_immediate_0080d9b0(bytes(u) + 0x838, throttle);
                const float axial = pc24(host.unit_reference_speed_0080fc30(u) * static_cast<double>(throttle));
                host.unit_controller_set_axial_speed_0092d770(at<void*>(u, 0x1018), axial);
            }
            found = host.scene_property_bag_find_008f2260(bag(), "CamoColor");
            at<std::int32_t>(u, 0x115C) = found != nullptr ? at<std::int32_t>(found, 0xC) : 0;
            found = host.scene_property_bag_find_008f2260(bag(), "CamoColorGun");
            if (found != nullptr) {
                // The sub-bag's R/G/B/A records are read without a null test.
                void* const colour = at<void*>(found, 0xC);
                at<float>(u, 0x1160) = bag_number(host.scene_property_bag_find_008f2260(colour, "R"));
                at<float>(u, 0x1164) = bag_number(host.scene_property_bag_find_008f2260(colour, "G"));
                at<float>(u, 0x1168) = bag_number(host.scene_property_bag_find_008f2260(colour, "B"));
                at<float>(u, 0x116C) = bag_number(host.scene_property_bag_find_008f2260(colour, "A"));
            } else {
                at<float>(u, 0x1160) = 0.5f;
                at<float>(u, 0x1164) = 0.5f;
                at<float>(u, 0x1168) = 0.5f;
                at<float>(u, 0x116C) = 1.0f;
            }
        } else if (kind == 2) {
            void* const authored = at<void*>(record, 8);
            at<std::uint32_t>(u, 0x58) = at<std::uint8_t>(authored, 0xC4);
            yard_launch = at<std::uint8_t>(authored, 0x11C);
            at<std::int32_t>(u, 0x115C) = at<std::int32_t>(authored, 0x128);
        }
    }
    // 00823704..00823780
    at<float>(u, 0x9D8) = 1.0f;
    at<float>(u, 0x9DC) = 1.0f;
    at<std::uint8_t>(u, 0x9E5) = 0;
    at<std::uint8_t>(u, 0x9E4) = 0;
    at<float>(u, 0x9E0) = 0.0f;
    at<std::int32_t>(u, 0x1130) = yard_launch != 0 ? 5 : 0;
    {
        void* const cls = at<void*>(u, 0x538);
        at<std::int32_t>(u, 0x10CC) = at<std::int32_t>(cls, 0x660);
        at<void*>(u, 0x10C8) = u;
        host.unit_0081e350(bytes(u) + 0x10B8, bytes(cls) + 0x664);
    }
    {
        void* const settings = host.game_settings_get_00424c40();
        const auto truncated = static_cast<std::int32_t>(at<float>(settings, 0x408));  // CVTTSS2SI
        host.leak_manager_init_0074f490(bytes(u) + 0x10D4, u, truncated);
    }

    if (at<void*>(u, 0xC0) != nullptr) {
        // 00823792..00823866: the part records of [[+360h]+160h] (8-byte
        // {record, value} entries, checked vector at +7Ch): the first whose
        // legacy name starts with "waterclip" stores its value in +1170h.
        if (at<void*>(u, 0x360) != nullptr) {
            const auto vector_of = [&]() -> std::uint8_t* { return at<std::uint8_t*>(at<void*>(u, 0x360), 0x160); };
            std::uint8_t* it = at<std::uint8_t*>(vector_of(), 0x80);
            std::uint8_t* const container = vector_of() + 0x7C;  // [ESP1+18h]
            if (it > at<std::uint8_t*>(container, 8)) host.crt_invalid_parameter_noinfo_00bf6713();
            for (;;) {
                std::uint8_t* const v = vector_of();
                std::uint8_t* const last = at<std::uint8_t*>(v, 0x84);
                if (at<std::uint8_t*>(v, 0x80) > last) host.crt_invalid_parameter_noinfo_00bf6713();
                if (container != v + 0x7C) host.crt_invalid_parameter_noinfo_00bf6713();
                if (it == last) break;
                if (!(it < at<std::uint8_t*>(container, 8))) host.crt_invalid_parameter_noinfo_00bf6713();
                alignas(4) std::uint8_t name[0x1C];  // std::string at [ESP1+7Ch]: +4 data, +18h capacity
                host.part_named_record_copy_legacy_name_00711c30(at<void*>(it, 0), name);
                if (host.stl_string_find_00814870(name, "waterclip", 0, 9) == 0) {
                    if (!(it < at<std::uint8_t*>(container, 8))) host.crt_invalid_parameter_noinfo_00bf6713();
                    at<std::int32_t>(u, 0x1170) = at<std::int32_t>(it, 4);
                }
                if (at<std::uint32_t>(name, 0x18) >= 0x10) {
                    host.crt_operator_delete_00bf65ac(at<void*>(name, 4));
                }
                if (!(it < at<std::uint8_t*>(container, 8))) host.crt_invalid_parameter_noinfo_00bf6713();
                it += 8;
            }
        }

        void* const record = at<void*>(u, 0xC0);
        const std::int32_t kind = at<std::int32_t>(record, 4);
        if (kind == 2) {
            // 00823882..008238AA: the authored record's skill (+B8h) and the
            // director's stance (+110h).
            void* const authored = at<void*>(record, 8);
            host.unit_vcall_128(u, at<std::int32_t>(authored, 0xB8));
            host.director_vcall_3c(at<void*>(u, 0x738), at<void*>(authored, 0x110));
        } else if (kind == 1) {
            // 008238B1..00823B6B: skill, navigator, director enables.
            if (at<std::int32_t>(at<void*>(u, 0xC0), 4) == 1) {
                const std::int32_t skill = host.property_bag_get_skill_00927a80(u);
                host.unit_vcall_128(u, skill);
            }
            if (at<void*>(u, 0x740) != nullptr) {
                void* const found = host.scene_property_bag_find_008f2260(bag(), "Navigator");
                if (found != nullptr && at<std::uint8_t>(found, 0xC) == 0) {
                    at<std::uint8_t>(at<void*>(u, 0x740), 0x11) = 0;
                }
            }
            const std::uint8_t artillery = bag_flag("ArtilleryDirector");  // [ESP1+34h]
            std::uint8_t torpedo;                                           // [ESP1+36h]
            {
                void* const found = host.scene_property_bag_find_008f2260(bag(), "TorpedoDirector");
                if (found != nullptr && at<std::uint8_t>(found, 0xC) == 0) {
                    torpedo = host.entity_vcall_5c_is_kind(u, 8) ? 1 : 0;
                } else {
                    torpedo = 1;
                }
            }
            const std::uint8_t anti_air = bag_flag("AADirector");     // [ESP1+35h]
            const std::uint8_t depth_charge = bag_flag("DCDirector");  // [ESP1+37h]
            std::int32_t stance = 1;                                   // EDI
            {
                void* const found = host.scene_property_bag_find_008f2260(bag(), "FireStance");
                if (found != nullptr) {
                    const std::int32_t value = at<std::int32_t>(found, 0xC);
                    stance = value >= 0 ? value : 1;
                }
            }
            const std::uint8_t player_commands = bag_flag("PlayerCommandsEnabled");  // BL
            const std::uint8_t torpedo_evasion = bag_flag("TorpedoEvasion");         // [ESP1+32h]
            const std::uint8_t ship_avoid = bag_flag("ShipCollAvoid");               // [ESP1+33h]
            const std::uint8_t land_avoid = bag_flag("LandCollAvoid");               // [ESP1+1Fh]
            at<std::uint8_t>(at<void*>(u, 0x73C), 0x20) = bag_flag("NavigatorAllowMaxDepth");
            alignas(4) std::uint8_t message[0x50];  // [ESP1+D8h]
            host.weapon_director_message_build_enables_007214c0(message, at<void*>(u, 0x738));
            at<std::uint8_t>(message, 4) = host.director_vcall_24(at<void*>(u, 0x738), stance) ? 1 : 0;
            at<std::uint8_t>(message, 5) = host.director_vcall_28(at<void*>(u, 0x738), stance) ? 1 : 0;
            at<std::uint8_t>(message, 0x40) = artillery;
            at<std::uint8_t>(message, 0x41) = anti_air;
            at<std::uint8_t>(message, 0x42) = torpedo;
            at<std::uint8_t>(message, 0x43) = depth_charge;
            at<std::uint8_t>(message, 0x44) = torpedo_evasion;
            at<std::uint8_t>(message, 0x45) = ship_avoid;
            at<std::uint8_t>(message, 0x46) = land_avoid;
            at<std::uint32_t>(message, 8) = player_commands != 0 ? 0x1FFu : 0u;
            at<std::uint8_t>(message, 0x47) = 0;
            at<float>(message, 0x48) = 0.0f;
            at<float>(message, 0x4C) = at<float>(u, 0x980);
            host.director_vcall_3c(at<void*>(u, 0x738), message);
        }
    }

    // 00823B6D..00823BEC: a ship node (+30h)+4A4h copies the class screw and
    // rudder blocks (+594h..+5A8h -> +1054h..+1068h) and binds them.
    if (at<std::uint8_t>(at<void*>(u, 0x30), 0x4A4) != 0) {
        void* cls = at<void*>(u, 0x538);
        at<float>(u, 0x1054) = at<float>(cls, 0x594);
        at<float>(u, 0x1058) = at<float>(cls, 0x598);
        at<float>(u, 0x105C) = at<float>(cls, 0x59C);
        cls = at<void*>(u, 0x538);
        at<float>(u, 0x1060) = at<float>(cls, 0x5A0);
        at<float>(u, 0x1064) = at<float>(cls, 0x5A4);
        at<float>(u, 0x1068) = at<float>(cls, 0x5A8);
        host.ship_unit_bind_screw_and_rudder_nodes_008346e0(u);
        host.unit_part_310_vcall_0c(bytes(u) + 0x310);
    } else {
        host.unit_00834690(u);
    }
    // 00823BEC..00823C38
    at<std::int32_t>(u, 0x1134) = 0;
    at<std::int32_t>(u, 0x1138) = 0;
    at<std::int32_t>(u, 0x113C) = 0;
    at<std::int32_t>(u, 0x1140) = 0;
    at<std::int32_t>(u, 0x1144) = 0;
    at<std::int32_t>(u, 0x1148) = 0;
    at<std::int32_t>(u, 0x1134) = static_cast<std::int32_t>(at<float>(host.game_settings_get_00424c40(), 0x46C));
    at<float>(u, 0x114C) = 0.0f;
    at<float>(u, 0x1158) = 0.0f;

    // 00823C40..00823E9A: one entity path per class path record ([+538h]+6CCh
    // checked vector of 10h-byte records whose +4h..+8h hold 0Ch-byte points),
    // pushed into +1178h and started (vtable +A0h).
    for (std::uint32_t index = 0;; ++index) {
        {
            void* const cls = at<void*>(u, 0x538);
            std::uint8_t* const begin = at<std::uint8_t*>(cls, 0x6D0);
            if (begin == nullptr) break;
            const std::int32_t n = static_cast<std::int32_t>(at<std::uint8_t*>(cls, 0x6D4) - begin) >> 4;
            if (!(index < static_cast<std::uint32_t>(n))) break;
        }
        void* const block = host.crt_operator_new_00bf55be(0x19C);
        host.crt_memset_00bf79f0(block, 0, 0x19C);
        void* const path = block != nullptr ? host.entity_path_construct_0080d690(block) : nullptr;  // [ESP1+18h]
        float identity[16];
        make_identity(identity);
        void* path_copy = path;  // [ESP1+28h]
        host.path_vcall_98(path, u, at<void*>(u, 0x30), identity);
        const auto checked_entry = [&]() -> std::uint8_t* {
            std::uint8_t* const list = bytes(at<void*>(u, 0x538)) + 0x6CC;
            std::uint8_t* const begin = at<std::uint8_t*>(list, 4);
            if (begin == nullptr ||
                !(index < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(list, 8) - begin) >> 4))) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            return at<std::uint8_t*>(list, 4) + index * 16u;
        };
        std::uint32_t j = 0;
        std::uint32_t offset = 0;  // [ESP1+14h]
        for (;;) {
            std::uint8_t* entry = checked_entry();
            std::uint8_t* points = at<std::uint8_t*>(entry, 4);
            if (points == nullptr) break;
            if (!(j < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(entry, 8) - points) / 12))) {
                break;
            }
            entry = checked_entry();
            points = at<std::uint8_t*>(entry, 4);
            if (points == nullptr ||
                !(j < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(entry, 8) - points) / 12))) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            host.path_points_vcall_08(bytes(path) + 0x170, at<std::uint8_t*>(entry, 4) + offset);
            ++j;
            offset += 12;
        }
        // 00823E25..00823E84: push_back into the pointer vector +1178h
        // ({+117Ch first, +1180h last, +1184h end}).
        std::uint8_t* const first = at<std::uint8_t*>(u, 0x117C);
        const std::int32_t size = first != nullptr ? static_cast<std::int32_t>(at<std::uint8_t*>(u, 0x1180) - first) >> 2 : 0;
        if (first != nullptr &&
            static_cast<std::uint32_t>(size) <
                static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(u, 0x1184) - first) >> 2)) {
            std::uint8_t* const last = at<std::uint8_t*>(u, 0x1180);
            at<void*>(last, 0) = path;
            at<std::uint8_t*>(u, 0x1180) = last + 4;
        } else {
            std::uint8_t* const last = at<std::uint8_t*>(u, 0x1180);
            if (first > last) host.crt_invalid_parameter_noinfo_00bf6713();
            alignas(4) std::uint8_t out[8];  // [ESP1+7Ch]
            host.pointer_vector_insert_one_0081da30(bytes(u) + 0x1178, out, bytes(u) + 0x1178, last, &path_copy);
        }
        host.path_vcall_a0(path);
    }

    // 00823E9F..00823F09: units not marked +5Dh with class records at +6E0h
    // (50h bytes) or +700h (1Ch bytes) register with [[00E188A8]+21D0h].
    if (at<std::uint8_t>(u, 0x5D) == 0) {
        void* const cls = at<void*>(u, 0x538);
        bool any = false;
        std::uint8_t* const a = at<std::uint8_t*>(cls, 0x6E0);
        if (a != nullptr && static_cast<std::int32_t>(at<std::uint8_t*>(cls, 0x6E4) - a) / 0x50 != 0) {
            any = true;
        } else {
            std::uint8_t* const b = at<std::uint8_t*>(cls, 0x700);
            if (b != nullptr && static_cast<std::int32_t>(at<std::uint8_t*>(cls, 0x704) - b) / 0x1C != 0) any = true;
        }
        if (any) {
            host.unit_manager_004a6830(at<void*>(host.global_00e188a8(), 0x21D0), u);
        }
    }

    // 00823F0C..008240BA: the bow wave pair +9F0h/+9F4h from class +5ACh with
    // the class matrices +5B0h and +5F0h, when the class offset +5E0h..+5E8h is
    // not zero (UCOMISS "!=", NaN counts as non-zero).
    {
        void* const cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0x5AC) != nullptr &&
            (at<float>(cls, 0x5E0) != 0.0f || at<float>(cls, 0x5E4) != 0.0f || at<float>(cls, 0x5E8) != 0.0f)) {
            create_parented(0x9F0, at<void*>(cls, 0x5AC), &at<float>(cls, 0x5B0));
            void* const cls2 = at<void*>(u, 0x538);
            create_parented(0x9F4, at<void*>(cls2, 0x5AC), &at<float>(cls2, 0x5F0));
        }
    }

    // 008240CB..008245EB: the wake +9E8h from class +630h in the effect system.
    // With a positive body-axis speed it is placed 10 and 9 speed units behind
    // along the frame's row 2 (+CCh) and stepped (0.1, 1.0), then at the unit
    // position (9.0); +50h = 1 / (modifier * +9C0h), +54h = speed.
    if (at<void*>(at<void*>(u, 0x538), 0x630) != nullptr) {
        float identity[16];
        make_identity(identity);
        create_in_system(0x9E8, at<void*>(at<void*>(u, 0x538), 0x630), identity);
        if (at<void*>(u, 0x9E8) != nullptr) {
            const float speed = static_cast<float>(host.unit_controller_body_axis_speed_0092d730(at<void*>(u, 0x1018)));
            if (speed > 0.0f) {
                at<float>(at<void*>(u, 0x9E8), 0x54) = speed;
                float m = modifier();
                float scaled = m * at<float>(u, 0x9C0);
                at<float>(at<void*>(u, 0x9E8), 0x50) = 1.0f / scaled;
                refresh_world();
                const float* const w = &at<float>(u, 0xCC);
                const float s1 = pc24(static_cast<double>(speed) * 10.0);
                const float a0 = s1 * w[8];
                const float a1 = w[9] * s1;
                const float a2 = s1 * w[10];
                refresh_world();
                const float* const pos = &at<float>(u, 0xFC);
                float p[3] = {pos[0] - a0, pos[1] - a1, pos[2] - a2};  // [ESP1+7Ch]
                host.point_effect_set_world_point_004842c0(at<void*>(u, 0x9E8), p);
                host.effect_update_children_00867790(at<void*>(u, 0x9E8), 0.1f,
                                                     at<void*>(host.global_00e188a8(), 0x19FC));
                refresh_world();
                const float s2 = pc24(static_cast<double>(speed) * 9.0);
                const float b0 = s2 * w[8];
                const float b1 = w[9] * s2;
                const float b2 = s2 * w[10];
                refresh_world();
                float q[3] = {pos[0] - b0, pos[1] - b1, pos[2] - b2};
                host.point_effect_set_world_point_004842c0(at<void*>(u, 0x9E8), q);
                host.effect_update_children_00867790(at<void*>(u, 0x9E8), 1.0f,
                                                     at<void*>(host.global_00e188a8(), 0x19FC));
                at<float>(at<void*>(u, 0x9E8), 0x54) = speed;
                m = modifier();
                scaled = m * at<float>(u, 0x9C0);
                at<float>(at<void*>(u, 0x9E8), 0x50) = 1.0f / scaled;
                refresh_world();
                host.point_effect_set_world_point_004842c0(at<void*>(u, 0x9E8), pos);
                host.effect_update_children_00867790(at<void*>(u, 0x9E8), 9.0f,
                                                     at<void*>(host.global_00e188a8(), 0x19FC));
            }
        }
    }

    // 008245F4..008247B2: the stern wake +9ECh from class +634h; +50h is written
    // without a null test; stepped by 10.0 when the speed is positive.
    if (at<void*>(at<void*>(u, 0x538), 0x634) != nullptr) {
        float identity[16];
        make_identity(identity);
        create_in_system(0x9EC, at<void*>(at<void*>(u, 0x538), 0x634), identity);
        const float m = modifier();
        const float scaled = m * at<float>(u, 0x9C0);
        at<float>(at<void*>(u, 0x9EC), 0x50) = 1.0f / scaled;
        if (host.unit_controller_body_axis_speed_0092d730(at<void*>(u, 0x1018)) > 0.0 &&
            at<void*>(u, 0x9EC) != nullptr) {
            host.effect_update_children_00867790(at<void*>(u, 0x9EC), 10.0f,
                                                 at<void*>(host.global_00e188a8(), 0x19FC));
        }
    }

    // 008247B5..00824905: four effects +A00h..+A0Ch from class +65Ch.
    if (at<void*>(at<void*>(u, 0x538), 0x65C) != nullptr) {
        for (std::size_t slot = 0xA00; slot < 0xA10; slot += 4) {
            float identity[16];
            make_identity(identity);
            create_in_system(slot, at<void*>(at<void*>(u, 0x538), 0x65C), identity);
        }
    }

    // 0082490B..00824B3C: one burst effect per class point record ([+538h]
    // +678h, count +67Ch, 10h bytes {x, y, z, id}) at that translation, kept in
    // the live list +A14h; its index (+A18h - 1) goes to the first of the 20
    // controller lists ([+1018h]+9Ch + k*20h) whose class id list
    // (+148h + k*30h) holds the record id.
    {
        void* const cls = at<void*>(u, 0x538);
        if (at<void*>(cls, 0x674) != nullptr) {
            std::uint8_t* rec = at<std::uint8_t*>(cls, 0x678);
            std::uint8_t* const end = rec + at<std::int32_t>(cls, 0x67C) * 16;
            while (rec != end) {
                float m[16];  // [ESP1+98h]
                make_identity(m);
                m[12] = at<float>(rec, 0);
                m[13] = at<float>(rec, 4);
                m[14] = at<float>(rec, 8);
                void* const parent = at<void*>(u, 0x4A4);
                void* const arg = copy_arg(at<void*>(at<void*>(u, 0x538), 0x674));
                void* created = nullptr;  // [ESP1+2Ch]
                host.point_effect_create_with_parent_matrix_008687c0(&created, parent, arg, m, 1, 0, 0);
                if (created != nullptr) {
                    host.live_effect_references_append_0074d780(bytes(u) + 0xA14, &created);
                }
                std::uint8_t* lists = bytes(at<void*>(u, 0x1018)) + 0x9C;
                std::uint8_t* ids = bytes(at<void*>(u, 0x538)) + 0x148;
                bool placed = false;  // [ESP1+1Fh]
                for (std::int32_t k = 0; k < 0x14 && !placed; ++k) {
                    for (std::uint32_t j = 0;; ++j) {
                        std::uint8_t* const first = at<std::uint8_t*>(ids, 4);
                        if (first == nullptr ||
                            !(j < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(ids, 8) - first) >> 2))) {
                            break;
                        }
                        if (at<std::int32_t>(first, j * 4) == at<std::int32_t>(rec, 0xC)) {
                            std::int32_t effect_index = at<std::int32_t>(u, 0xA18) - 1;  // [ESP1+20h]
                            host.dword_vector_push_back_00442190(lists, &effect_index);
                            placed = true;
                            break;
                        }
                    }
                    ids += 0x30;
                    lists += 0x20;
                }
                if (created != nullptr) {
                    release(created);
                    created = nullptr;
                }
                rec += 16;
            }
        }
    }
}

}  // namespace bsp
