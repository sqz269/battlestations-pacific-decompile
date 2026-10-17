// Segment 59 TorpedoBot / DepthChargeBot slot bodies (packet s53_seg59_bots,
// docs/SHIP_AI_OPEN_ITEMS.md 264), and the seg 59 helpers 008FB890 / 00902290 (packet
// s54_seg59_helpers, 268). Transcribed from the disk listing.

#include "bsp/seg59_bots_s53.hpp"
#include "bsp/seg59_ship_lead_s54.hpp"

#include "bsp/camera_multiply.hpp"
#include "bsp/native_particle_axial_loading.hpp"

#include <cmath>

#include <cstddef>
#include <cstring>

namespace bsp {

namespace {

template <class T>
T load(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(base) + offset, sizeof value);
    return value;
}

template <class T>
void store(void* base, std::size_t offset, T value) {
    std::memcpy(static_cast<unsigned char*>(base) + offset, &value, sizeof value);
}

// 008FF2B8..008FF2DD / 008FF508..008FF52D: the owner's role-0 slot +1ACh; slot 8
// or an AI-held slot sends gun->vtable[1E8h](0).
void release_gun_when_ai_held(void* bot, Seg59BotHost& host) {
    void* const owner = load<void*>(bot, 0x50);
    const std::int32_t slot = load<std::int32_t>(owner, 0x1ac);
    if (slot != 8 && !host.party_slot_is_ai_held_00927f10(slot)) return;
    host.gun_vcall_1e8(load<void*>(bot, 0x58), 0);
}

// 008FF31E..008FF329 / 008FC06D..008FC076: +58h = +50h, +5Ch = 0.0f (XORPS / MOVSS).
void cache_owner(void* bot) {
    store<void*>(bot, 0x58, load<void*>(bot, 0x50));
    store<float>(bot, 0x5c, 0.0f);
}

}  // namespace

void seg59_torpedo_bot_clear_fire_target_008ff2b0(void* bot, Seg59BotHost& host) {
    host.gun_bot_clear_fire_target_0072bd50(bot);   // 008FF2B3
    release_gun_when_ai_held(bot, host);
    store<std::uint8_t>(bot, 0x6c, 0);               // 008FF2DF, on both paths
}

void seg59_depth_charge_bot_clear_fire_target_008ff500(void* bot, Seg59BotHost& host) {
    host.gun_bot_clear_fire_target_0072bd50(bot);   // 008FF503
    release_gun_when_ai_held(bot, host);
}

void seg59_torpedo_bot_attach_008ff310(void* bot, void* node, Seg59BotHost& host) {
    host.gun_bot_attach_008fbc80(bot, node);        // 008FF319
    cache_owner(bot);
    void* const owner = load<void*>(bot, 0x50);
    // 008FF32E..008FF34A: +64h = owner when it answers IsKindOf(08h), else 0.
    if (owner != nullptr && host.entity_vcall_5c_is_kind_of(owner, 8)) {
        store<void*>(bot, 0x64, owner);
        return;
    }
    store<void*>(bot, 0x64, nullptr);
}

void seg59_depth_charge_bot_attach_008fc060(void* bot, void* node, Seg59BotHost& host) {
    host.gun_bot_attach_008fbc80(bot, node);        // 008FC068
    cache_owner(bot);
}

bool seg59_entity_is_live_other_party_008fb890(void* gun, void* target, Seg59BotHost& host) {
    if (target == nullptr) return false;            // 008FB890..008FB892, 008FB8C6 XOR AL,AL
    // 008FB895: BL = 0 is "still good"; any failure sets BL = 1 (008FB8BB) and the
    // answer is SETZ on BL (008FB8C1).
    if (load<std::uint8_t>(target, 0x5d) != 0) return false;   // 008FB897, the dead flag
    if (gun == nullptr) return false;                           // 008FB89C
    void* const parent = load<void*>(gun, 0x3c);               // 008FB8A0
    if (parent == nullptr) return false;                        // 008FB8A3
    if (load<std::uint8_t>(parent, 0x5d) != 0) return false;   // 008FB8A7
    // 008FB8AC..008FB8B9: ECX = gun+54h, EDX = target+54h; a zero answer fails.
    return host.party_relative_to_00803510(load<void*>(gun, 0x54),
                                           load<void*>(target, 0x54)) != 0;
}

}  // namespace bsp

// ---------------------------------------------------------------------------
// 00902290, the turning-ship lead (packet s54_seg59_helpers, docs/SHIP_AI_OPEN_ITEMS.md
// 268). Transcribed from the disk listing (bsp.py disasm-raw 00902290 --length 0x690);
// kept in this file so the build needs no new source entry. Constants from the PE:
// 00D0DE84 = 0.05f, 00D7A238 = 0.01f, qword 00D7A328 = 4.0, qword 00D7A258 = 0.0,
// 00D7A208 = -0.0f, 00D7A24C = 1.0f, qword 00CE3820 = 1e-10.

namespace bsp {

namespace ship_lead_00902290 {

// 0042B2F0 BSP_Vector3_LengthFloatThreshold: (y*y + x*x) + z*z as a float, the CRT
// sqrt when strictly above the qword 1e-10 (00CE3820), else 0 (unordered included).
float length_0042b2f0(float x, float y, float z) {
    const float sum = (y * y + x * x) + z * z;
    if (static_cast<double>(sum) > 1e-10) return static_cast<float>(std::sqrt(sum));
    return 0.0f;
}

CameraMatrix translation(float x, float y, float z) {
    CameraMatrix m{1.0f, 0.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f, 0.0f,
                   0.0f, 0.0f, 1.0f, 0.0f,
                   x,    y,    z,    1.0f};
    return m;
}

void raw_target(const ShipTurnLeadTarget& target, float out_aim[3]) {
    // 009028BF..009028E1: the target position itself (+FCh..+104h, read at 00902399).
    out_aim[0] = target.pose[12];
    out_aim[1] = target.pose[13];
    out_aim[2] = target.pose[14];
}

}  // namespace ship_lead_00902290

using namespace ship_lead_00902290;

ShipTurnLeadResult seg59_ship_turn_lead_00902290(const float shooter[3],
    const ShipTurnLeadTarget& target, float speed, const float shooter_vel[3],
    float out_aim[3], CameraMatrix& out_matrix, float* out_distance, ShipTurnLeadHost& host) {
    ShipTurnLeadResult result;
    // 00902299..009022C1: f = +9DCh * +984h; FUCOMIP against 0, LAHF / TEST AH,44h / JP:
    // only an exact (ordered) zero falls through.
    const float f = target.field_9dc * target.field_984;
    if (f == 0.0f) {
        // 009022C7..0090234D: an identity built on the stack, copied by 004134F0.
        out_matrix = translation(0.0f, 0.0f, 0.0f);
        // 00902352..00902379: 00901C20(shooter, target)(speed, shooter_vel, out_aim, out_distance).
        host.intercept_solution_00901c20(shooter, speed, shooter_vel, out_aim, out_distance);
        result.via_intercept = true;
        return result;
    }
    const float tx = target.pose[12];
    const float ty = target.pose[13];
    const float tz = target.pose[14];
    // 009023CE..009023F3: the relative velocity in the horizontal plane (+4Ch = 0).
    const float rvx = target.velocity[0] - shooter_vel[0];
    const float rvz = target.velocity[2] - shooter_vel[2];
    // 009023F7..00902413, 00902417: D = target - shooter, L = |D|.
    const float dx = tx - shooter[0];
    const float dy = ty - shooter[1];
    const float dz = tz - shooter[2];
    const float length = length_0042b2f0(dx, dy, dz);
    float t = 0.0f;
    bool solved = false;
    // 00902420..0090242A: FLD1 / FCOMI; L <= 1 (or unordered) takes the raw target.
    if (length > 1.0f) {
        const float nx = dx / length;   // 00902430..0090244A
        const float ny = dy / length;
        const float nz = dz / length;
        // 0090244E..00902474: the closing speed along D, the y term a literal 0.
        const float closing = (rvx * nx + ny * 0.0f) + nz * rvz;
        // 00902478..0090248A: |Rv|^2 with the same 0.
        const float vv = (rvx * rvx + 0.0f * 0.0f) + rvz * rvz;
        // 0090248E..009024A2: the lateral part, stored as a float.
        const float lateral = vv - closing * closing;
        if (0.01f > lateral) {
            // 009024AA..009024B4 FCOMIP / JBE: 00D7A238 > lateral (ordered) is this arm,
            // anything else (unordered included) the quadratic. t = L / (speed - closing).
            t = length / (speed - closing);
            // 009024C5..009024CF: 0 > t (ordered) leaves; otherwise 009025C5 tests t > 0.
            solved = !(0.0f > t) && t > 0.0f;
        } else {
            // 009024E3..00902591: A t^2 + B t + L^2 (in L units), the roots as floats.
            const float qa = vv - speed * speed;
            const float qb = (length + length) * closing;
            const float l2 = length * length;
            const float disc = qb * qb - static_cast<float>(qa * 4.0f) * l2;
            float t1 = 0.0f;
            float t2 = 0.0f;
            if (disc >= 0.0f) {   // 00902537..00902541: JB on disc < 0 or unordered
                const float root = static_cast<float>(std::sqrt(disc));   // 00902543 CRT sqrt
                t1 = (-qb + root) / (qa + qa);
                t2 = (-qb - root) / (qa + qa);
            }
            // 00902595..009025B6: t1, unless t1 < 0, or t2 >= 0 and t1 > t2 (COMISS / JB
            // and FCOMIP / JBE: unordered keeps t1).
            t = t1;
            if (0.0f > t1) {
                t = t2;
            } else if (!(t2 < 0.0f) && t1 > t2) {
                t = t2;
            }
            // 009025BC..009025C8: 0 > t leaves; t <= 0 or unordered leaves.
            solved = !(0.0f > t) && t > 0.0f;
        }
    }
    if (!solved) {
        raw_target(target, out_aim);
    } else {
        // 009025CE..009025EF: t' = 0.05 + t.
        const float lead_time = 0.05f + t;
        // 009025F3..00902623: R = ([+538h]+520h / f) * 0082E890(|+980h|).
        const float curve = host.rudder_curve_denominator_0082e890(std::fabs(target.field_980));
        const float radius = (target.class_520 / f) * curve;
        // 00902630..0090268C: the turn centre, pose row 2 (+ECh x, +F4h z) turned a
        // quarter: C = T + R * (-fz, 0, fx); the y add is the qword 0.0.
        const float off_x = -target.pose[10] * radius;
        const float off_z = radius * target.pose[8];
        const float cx = off_x + tx;
        const float cy = static_cast<float>(static_cast<double>(ty) + 0.0);
        const float cz = off_z + tz;
        // 00902690..009026AC: the yaw turned through in t', the arc run at the
        // forward speed over the turn radius: -(speed * t' / R).
        float angle = (target.speed_38 * lead_time) / radius;
        angle = -angle;
        // 009026B0..009026D3: 00B646E0 into a scratch, copied to the scratch and to
        // out_matrix (004134F0 twice).
        static const volatile float kNegativeZero = -0.0f;   // 00D7A208
        static const volatile float kOne = 1.0f;             // 00D7A24C
        CameraMatrix rotation{};
        build_native_particle_rotation_y_00b646e0(rotation.data(), &angle, &kNegativeZero, &kOne);
        out_matrix = rotation;
        // 009026D8..00902818: T(C) and T(-0.0 - C) built on the stack.
        const CameraMatrix to_centre = translation(cx, cy, cz);
        const CameraMatrix from_centre = translation(-0.0f - cx, -0.0f - cy, -0.0f - cz);
        // 00902821..00902857: ((pose * T(-C)) * Ry) * T(C) through 00413920 three times.
        CameraMatrix m1{}, m2{}, m3{};
        multiply_camera_matrices_00413920(m1, target.pose, from_centre);
        multiply_camera_matrices_00413920(m2, m1, rotation);
        multiply_camera_matrices_00413920(m3, m2, to_centre);
        out_aim[0] = m3[12];   // 0090285C..0090287C
        out_aim[1] = m3[13];
        out_aim[2] = m3[14];
        // 0090287F..009028B8: less the shooter's own travel over t'.
        const float sx = shooter_vel[0] * lead_time;
        const float sy = shooter_vel[1] * lead_time;
        const float sz = lead_time * shooter_vel[2];
        out_aim[0] = out_aim[0] - sx;
        out_aim[1] = out_aim[1] - sy;
        out_aim[2] = out_aim[2] - sz;
        result.led = true;
        result.time = lead_time;
    }
    // 009028E6..00902913: out_distance = |out_aim - shooter|.
    *out_distance = length_0042b2f0(out_aim[0] - shooter[0], out_aim[1] - shooter[1],
                                    out_aim[2] - shooter[2]);
    return result;
}

}  // namespace bsp
