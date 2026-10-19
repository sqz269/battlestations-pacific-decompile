// cc13_seg67n: the segment-67 rows oct18b-nC left open (packet cc13-seg67n).
// See include/bsp/cc13_seg67n.hpp and docs/CC13_SEG67N.md. Hypothesis names,
// build-tested only, nothing wired.

#include "bsp/cc13_seg67n.hpp"

#include <cmath>
#include <cstring>

namespace bsp {
namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg67n_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return static_cast<std::uint8_t*>(base) + offset;
}

void* ptr_at(void* base, std::size_t offset) {
    return reinterpret_cast<void*>(at<std::uintptr_t>(base, offset));
}

std::uint32_t float_bits(float f) {
    std::uint32_t u = 0;
    std::memcpy(&u, &f, sizeof u);
    return u;
}

// The shared wait-value shape: task+310h == task+follow -> JMP 009BE3E0 with
// ECX = that state, else JMP 0099B720 with ECX = task.
bool state_is(void* task, std::size_t state_offset) {
    return at<std::uintptr_t>(task, 0x310) == reinterpret_cast<std::uintptr_t>(ptr(task, state_offset));
}

}  // namespace

// 0099A810 (0099A810-0099A824): CMP [ECX+5Ch],0; JZ -> XOR EAX,EAX; RET. Else
// ECX = *[ECX+58h] (element 0 of the task array), JMP [vtable+8h].
std::uint32_t pilot_bot_head_task_slot08_0099a810(Cc13Seg67nHost& host, void* bot) {
    if (at<std::uint32_t>(bot, 0x5C) != 0) {
        void* const head = *static_cast<void**>(ptr_at(bot, 0x58));
        return host.task_vcall_08(head);
    }
    return 0;
}

// 0099C2A0 (0099C2A0-0099C2BD): ECX = [task+310h]; null -> 0. state->vtable[+18h]()
// AL nonzero -> EAX = 1, else 0. No guard on task itself (007BBB53 can pass 0).
std::uint32_t bot_task_current_state_query18_0099c2a0(Cc13Seg67nHost& host, void* task) {
    void* const state = ptr_at(task, 0x310);
    if (state != nullptr && host.state_vcall_18(state)) return 1;
    return 0;
}

// ---- one-line task slots (each body is its whole listing: MOV EAX,imm; RET) ----
std::uint32_t bot_task_depth_charge_slot10_009a54f0() noexcept { return 4u; }
std::uint32_t bot_task_dogfight_slot10_009a99f0() noexcept { return 7u; }
std::uint32_t bot_task_drop_kamikaze_slot10_009addd0() noexcept { return 4u; }
std::uint32_t bot_task_kamikaze_slot10_009af070() noexcept { return 8u; }
std::uint32_t bot_task_land_slot10_009b33b0() noexcept { return 2u; }
std::uint32_t bot_task_level_bomb_slot10_009b7bc0() noexcept { return 4u; }
std::uint32_t bot_task_immelmann_slot10_009ba910() noexcept { return 0x0Au; }
std::uint32_t bot_task_gain_altitude_slot10_009baa80() noexcept { return 0x0Au; }
std::uint32_t bot_task_stop_slot10_009bac50() noexcept { return 0x0Bu; }
std::uint32_t bot_task_moveto_slot10_009c3160() noexcept { return 3u; }
std::uint32_t bot_task_strafe_slot10_009cc3e0() noexcept { return 6u; }
std::uint32_t bot_task_takeoff_slot10_009cf9f0() noexcept { return 1u; }
std::uint32_t bot_task_torpedo_slot10_009d3290() noexcept { return 5u; }
std::uint32_t bot_task_immelmann_factory_record_009ba8f0() noexcept { return kSeg67nImmelmannFactory; }
std::uint32_t bot_task_gain_altitude_factory_record_009baa70() noexcept {
    return kSeg67nGainAltitudeFactory;
}
std::uint32_t bot_task_stop_factory_record_009bac20() noexcept { return kSeg67nStopFactory; }
std::uint32_t bot_task_moveto_factory_record_009c3140() noexcept { return kSeg67nMoveToFactory; }
bool bot_task_stop_slot30_009bac40() noexcept { return false; }
bool bot_task_takeoff_slot24_009cfa30() noexcept { return true; }

// 009A54B0: MOV EAX,[ECX+48Ch]; RET.
void* bot_task_depth_charge_target_009a54b0(void* task) { return ptr_at(task, 0x48C); }

// 009A9960 (009A9960-009A996D): XORPS; MOVSS [ECX+4E0h],XMM0; MOV AL,1; RET 4.
// The argument is unread.
bool bot_task_dogfight_hit_notice_009a9960(void* task) {
    at<float>(task, 0x4E0) = 0.0f;
    return true;
}

// 009AA3B0 (009AA3B0-009AA404): ESI = [task+4ACh], else [task+4C4h]; both null ->
// byte out+0Ch = 0. Else 00414DB0(ESI) when byte ESI+C8h is 0, then the stores
// in the image's order: out+0 = ESI+FCh, byte out+0Ch = 1, out+4 = +100h,
// out+8 = +104h (FLD/FSTP float copies).
void* bot_task_dogfight_target_record_009aa3b0(Cc13Seg67nHost& host, void* task, void* out) {
    void* entity = ptr_at(task, 0x4AC);
    if (entity == nullptr) {
        entity = ptr_at(task, 0x4C4);
        if (entity == nullptr) {
            at<std::uint8_t>(out, 0xC) = 0;
            return out;
        }
    }
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    at<float>(out, 0x0) = at<float>(entity, 0xFC);
    at<std::uint8_t>(out, 0xC) = 1;
    at<float>(out, 0x4) = at<float>(entity, 0x100);
    at<float>(out, 0x8) = at<float>(entity, 0x104);
    return out;
}

// 009ABBC0 (009ABBC0-009ABBD5): EAX = [ESP+4]; three FLD/FSTP float copies from
// ECX+4Ch/+50h/+54h; RET 4.
void* bot_point_at_4c_009abbc0(void* object, void* out) {
    at<float>(out, 0x0) = at<float>(object, 0x4C);
    at<float>(out, 0x4) = at<float>(object, 0x50);
    at<float>(out, 0x8) = at<float>(object, 0x54);
    return out;
}

// 009AE260 (009AE260-009AE288): task+50Ch, then task+5C0h, else 0099B720.
float bot_task_drop_kamikaze_wait_value_009ae260(Cc13Seg67nHost& host, void* task) {
    if (state_is(task, 0x50C)) return host.bot_state_follow_wait_value_009be3e0(ptr(task, 0x50C));
    if (state_is(task, 0x5C0)) return host.bot_state_follow_wait_value_009be3e0(ptr(task, 0x5C0));
    return host.bot_task_base_wait_value_0099b720(task);
}
// 009AF2F0 (009AF2F0-009AF305): task+524h, else 0099B720.
float bot_task_kamikaze_wait_value_009af2f0(Cc13Seg67nHost& host, void* task) {
    if (state_is(task, 0x524)) return host.bot_state_follow_wait_value_009be3e0(ptr(task, 0x524));
    return host.bot_task_base_wait_value_0099b720(task);
}
// 009C3550 (009C3550-009C3565): task+494h, else 0099B720 (moveto and attackmove).
float bot_task_moveto_wait_value_009c3550(Cc13Seg67nHost& host, void* task) {
    if (state_is(task, 0x494)) return host.bot_state_follow_wait_value_009be3e0(ptr(task, 0x494));
    return host.bot_task_base_wait_value_0099b720(task);
}

// 009CFA40 (009CFA40-009CFA5B): EAX = [task+404h]; null -> JMP 0099B720. EAX+3D0h
// == [task+3FCh] -> JMP 0099B720; else FLDZ; RET.
float bot_task_takeoff_wait_value_009cfa40(Cc13Seg67nHost& host, void* task) {
    void* const control = ptr_at(task, 0x404);
    if (control != nullptr && at<std::uint32_t>(control, 0x3D0) != at<std::uint32_t>(task, 0x3FC)) {
        return 0.0f;
    }
    return host.bot_task_base_wait_value_0099b720(task);
}

// 009B36C0 (009B36C0-009B36E9): [task+310h] == task+5D8h, +5F8h or +620h -> AL = 1.
bool bot_task_land_in_landing_states_009b36c0(void* task) {
    return state_is(task, 0x5D8) || state_is(task, 0x5F8) || state_is(task, 0x620);
}

// 009B9590 (009B9590-009B95C2): [task+2FCh] == 0 -> EAX = 2. Else ECX =
// [task+2FCh], CALL [vtable+114h], ECX = EAX, CALL 0071BE40, EAX == 00E08F88 ->
// SETZ -> EAX = 1 or 0.
std::uint32_t bot_task_stop_still_valid_009b9590(Cc13Seg67nHost& host, void* task) {
    void* const squadron = ptr_at(task, 0x2FC);
    if (squadron == nullptr) return 2;
    void* const director = host.object_vcall_114(squadron);
    return host.weapon_director_current_command_0071be40(director) == kSeg67nStopCommandClass ? 1u : 0u;
}

// 009BA9E0 (009BA9E0-009BAA16): 0099D060(task, archive); then archive->vtable[+0Ch]
// ({0, "stage" 00D20474}, {1, dword [task+3F8h]}), the dword read after the base save.
void bot_task_immelmann_save_009ba9e0(Cc13Seg67nHost& host, void* task, void* archive) {
    host.bot_task_save_base_0099d060(task, archive);
    host.archive_vcall_0c_write(archive, {0, kSeg67nStrStage},
                                {1, at<std::uint32_t>(task, 0x3F8)});
}

// 009BAB70 (009BAB70-009BABDF): 0099D060(task, archive); write({0, "target"
// 00CFD964}, {4, [task+3F8h]}); then write({0, "alt" 00D204F8}, {2, float
// [task+3FCh]}), each field read just before its write.
void bot_task_gain_altitude_save_009bab70(Cc13Seg67nHost& host, void* task, void* archive) {
    host.bot_task_save_base_0099d060(task, archive);
    host.archive_vcall_0c_write(archive, {0, kSeg67nStrTarget},
                                {4, at<std::uint32_t>(task, 0x3F8)});
    host.archive_vcall_0c_write(archive, {0, kSeg67nStrAlt},
                                {2, float_bits(at<float>(task, 0x3FC))});
}

// 009BAC30: JMP 0099D060 (ECX and the archive argument passed through).
void bot_task_stop_save_009bac30(Cc13Seg67nHost& host, void* task, void* archive) {
    host.bot_task_save_base_0099d060(task, archive);
}

// 009BE140: MOV EAX,[ECX+10h]; RET.
void* move_to_target_entity_009be140(void* target) { return ptr_at(target, 0x10); }

// ---- batch C ----
namespace {
constexpr double kImmelmannSpeedGain_00ce3d10 = 0.20000000298023224;  // 0.2f as a qword
constexpr float kSixthPi_00cec724 = 0.52359879f;       // bytes 92 0A 06 3F
constexpr double kFifty_00ce3938 = 50.0;
constexpr float kFortyDeg_00ce7d20 = 0.69813174f;      // bytes C3 B8 32 3F
constexpr float kTenDeg_00ce3990 = 0.17453294f;        // bytes C3 B8 32 3E
constexpr float kMinusThirdPi_00d20338 = -1.0471976f;  // bytes 92 0A 86 BF
constexpr float kMinusTenDeg_00ceca08 = -0.17453294f;  // bytes C3 B8 32 BE
constexpr float kPi_00d7a264 = 3.14159274f;            // bytes DB 0F 49 40
constexpr float kMinusZero_00d7a208 = -0.0f;           // bytes 00 00 00 80
constexpr double kHundred_00d7a220 = 100.0;
constexpr float kDebugTextX_00ce3930 = 20.0f;
constexpr float kDebugTextY_00ce4bc4 = 600.0f;
constexpr float kSphereRadius3_00ce3854 = 3.0f;
constexpr float kSphereRadius5_00ce3850 = 5.0f;
constexpr std::uint32_t kStrFollowFormat_00d209d8 = 0x00D209D8u;  // "follow: %.2f  %.2f"

// The function-local static colours the image builds once under a guard bit
// (00F8A56C green guarded by 00F8A57C, 00F8A558 red by 00F8A568, 00F8A544 white
// by 00F8A554, 00F8A6F0 yellow by 00F8A700). The guarded stores only ever write
// these values, so the reconstruction passes the constants.
constexpr float kGreen[4] = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr float kRed[4] = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr float kWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr float kYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};

float abs_bits(float f) {  // AND 7FFFFFFFh on the float's bits
    std::uint32_t u = float_bits(f) & 0x7FFFFFFFu;
    float r = 0.0f;
    std::memcpy(&r, &u, sizeof r);
    return r;
}

void refresh_pose_if_stale(Cc13Seg67nHost& host, void* entity) {
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
}
}  // namespace

// 009B9AF0 (009B9AF0-009B9C94), immelmann. u = task+2F4h, the stage task+3F8h.
//  - stage != 2: +294h = float(cos(u+C64h)) x u+C68h - u+B50h x 0.2 (qword 00CE3D10;
//    009B9B2E FCOS, 009B9B46 FSUBP DE E9 = ST1 - ST0, the decompiler agrees); stage 2:
//    +294h = 0.
//  - 009B9B58-009B9BA3: +298h = 1, +2D0h = 0, +288h = 0.0, +28Ch = 1, +2D8h = 0, +27Ch =
//    1.0, +280h = 1, +2ACh = 0.0, +2B0h = 1, +2DCh = 0.
//  - stage 0: |u+C68h| > pi/6 (COMISS/JBE, NaN falls through) and 50.0 > |u+C64h|
//    (FCOMIP/JBE, NaN falls through) -> +2A0h = 0.0, +2A4h = 1, +2D4h = 0 and RET 4
//    (no +2E4h store). Otherwise the stage becomes 1.
//  - stage != 0 or fell through: +2A0h = 1.0, +2A4h = 1, +2D4h = 0; u+C64h > 40 deg
//    -> stage 2; stage 2 and 10 deg > u+C64h -> 0099B690(task).
//  - 009B9C86: AND dword [task+2E4h], 0FBh (81 /4 imm32: bits 8-31 clear too).
// x87 at 24-bit precision (product and difference rounded to float); 53-bit is the
// alternative. FCOS runs at extended precision whatever the control word: std::cos
// in double then the float store can differ by an ulp in rare cases. No draw.
void bot_task_immelmann_tick_009b9af0(Cc13Seg67nHost& host, void* task, float) {
    void* unit = ptr_at(task, 0x2F4);
    if (at<std::int32_t>(task, 0x3F8) != 2) {
        const float speed = at<float>(unit, 0xB50);
        const float pitch = at<float>(unit, 0xC64);
        const float bank = at<float>(unit, 0xC68);
        const float c = static_cast<float>(std::cos(static_cast<double>(pitch)));
        const float roll_term = c * bank;
        const float speed_term = static_cast<float>(static_cast<double>(speed) * kImmelmannSpeedGain_00ce3d10);
        at<float>(task, 0x294) = roll_term - speed_term;
    } else {
        at<float>(task, 0x294) = 0.0f;
    }
    at<std::uint8_t>(task, 0x298) = 1;
    at<std::int32_t>(task, 0x2D0) = 0;
    at<float>(task, 0x288) = 0.0f;
    at<std::uint8_t>(task, 0x28C) = 1;
    at<std::int32_t>(task, 0x2D8) = 0;
    at<float>(task, 0x27C) = 1.0f;
    at<std::uint8_t>(task, 0x280) = 1;
    at<float>(task, 0x2AC) = 0.0f;
    at<std::uint8_t>(task, 0x2B0) = 1;
    at<std::int32_t>(task, 0x2DC) = 0;
    if (at<std::int32_t>(task, 0x3F8) == 0) {
        unit = ptr_at(task, 0x2F4);
        if (abs_bits(at<float>(unit, 0xC68)) > kSixthPi_00cec724
                && kFifty_00ce3938 > static_cast<double>(abs_bits(at<float>(unit, 0xC64)))) {
            at<float>(task, 0x2A0) = 0.0f;
            at<std::uint8_t>(task, 0x2A4) = 1;
            at<std::int32_t>(task, 0x2D4) = 0;
            return;
        }
        at<std::int32_t>(task, 0x3F8) = 1;
    }
    at<float>(task, 0x2A0) = 1.0f;
    at<std::uint8_t>(task, 0x2A4) = 1;
    at<std::int32_t>(task, 0x2D4) = 0;
    unit = ptr_at(task, 0x2F4);
    if (at<float>(unit, 0xC64) > kFortyDeg_00ce7d20) at<std::int32_t>(task, 0x3F8) = 2;
    if (at<std::int32_t>(task, 0x3F8) == 2 && kTenDeg_00ce3990 > at<float>(unit, 0xC64)) {
        host.bot_task_retire_0099b690(task);
    }
    at<std::uint32_t>(task, 0x2E4) &= 0xFBu;
}

// 009B9CA0 (009B9CA0-009B9E36), split s. u = task+2F4h, cls = task+2F8h.
//  - Head: +2B8h = +400h, byte +2B4h = 0, +2DCh = 1.
//  - stage 0 (009B9D79): +294h = +3FCh, +298h = 1, +2D0h = 0, +2A0h = 0.0, +2A4h = 1,
//    +2D4h = 0. r = 00438B10(pi, u+C68h) (pushed: [ESP] = pi, [ESP+4] = the bank);
//    |r| = r > 0 ? r : -0.0 - r (FCOMIP/JBE: 0 and NaN take -0.0 - r); t = float(cls+1A8h
//    / cls+1BCh), doubled (FADD ST0,ST0, exact); t > |r| (FCOMIP/JBE) -> stage 1.
//  - stage 1 or 2 ((stage - 1) <= 1 unsigned): +2A0h = 1.0, +2A4h = 1, +2D4h = 0;
//    009B94E0(task, stage == 1). Stage 1: -pi/3 > u+C64h -> AND, stage 2, RET 4.
//    Stage 2: u+C64h > -10 deg -> 0099B690(task), AND, RET 4.
//  - Every exit ANDs dword [task+2E4h] with 0FBh (81 /4 imm32).
// The quotient is one rounding under either control word (a float quotient
// computed in double rounds to float exactly once); no other arithmetic. No draw.
void bot_task_split_s_tick_009b9ca0(Cc13Seg67nHost& host, void* task, float) {
    at<float>(task, 0x2B8) = at<float>(task, 0x400);
    at<std::uint8_t>(task, 0x2B4) = 0;
    at<std::int32_t>(task, 0x2DC) = 1;
    const std::int32_t stage = at<std::int32_t>(task, 0x3F8);
    if (stage == 0) {
        at<float>(task, 0x294) = at<float>(task, 0x3FC);
        at<std::uint8_t>(task, 0x298) = 1;
        at<std::int32_t>(task, 0x2D0) = 0;
        at<float>(task, 0x2A0) = 0.0f;
        at<std::uint8_t>(task, 0x2A4) = 1;
        at<std::int32_t>(task, 0x2D4) = 0;
        const float bank = at<float>(ptr_at(task, 0x2F4), 0xC68);
        const float r = host.math_subtract_wrapped_angle_00438b10(kPi_00d7a264, bank);
        const float abs_r = r > 0.0f ? r : kMinusZero_00d7a208 - r;
        void* const cls = ptr_at(task, 0x2F8);
        const float q = at<float>(cls, 0x1A8) / at<float>(cls, 0x1BC);
        const float t = q + q;
        if (t > abs_r) at<std::int32_t>(task, 0x3F8) = 1;
    } else if (static_cast<std::uint32_t>(stage - 1) <= 1u) {
        at<float>(task, 0x2A0) = 1.0f;
        at<std::uint8_t>(task, 0x2A4) = 1;
        at<std::int32_t>(task, 0x2D4) = 0;
        host.bot_task_split_s_roll_step_009b94e0(task, at<std::int32_t>(task, 0x3F8) == 1);
        if (at<std::int32_t>(task, 0x3F8) == 1) {
            if (kMinusThirdPi_00d20338 > at<float>(ptr_at(task, 0x2F4), 0xC64)) {
                at<std::uint32_t>(task, 0x2E4) &= 0xFBu;
                at<std::int32_t>(task, 0x3F8) = 2;
                return;
            }
        } else if (at<float>(ptr_at(task, 0x2F4), 0xC64) > kMinusTenDeg_00ceca08) {
            host.bot_task_retire_0099b690(task);
            at<std::uint32_t>(task, 0x2E4) &= 0xFBu;
            return;
        }
    }
    at<std::uint32_t>(task, 0x2E4) &= 0xFBu;
}

// 009BE590 (009BE590-009BEA9E), the follow state's debug draw. approach = [state+4h],
// unit = [approach+4h].
//  - byte state+85h == 0: unit pose refreshed; 0085F990(unit+FCh, state+60h, white,
//    green) at 009BE67F; state+5Ch > 0.0 (COMISS/JBE) -> 00860710(state+50h, green,
//    state+5Ch) at 009BE6D8.
//  - 009BE6DD-009BE714: 004C7070([00E188A8], 20.0, 600.0, "follow: %.2f  %.2f",
//    double(state+90h), double(state+94h)); +90h is read first into the lower slot.
//  - 00860BE0(state+44h, red, 3.0, red, 1) at 009BE79F; unit pose refreshed;
//    0085F990(unit+FCh, state+44h, white, red) at 009BE87D.
//  - 00860BE0(state+30h, green, 5.0, green, 1) at 009BE905.
//  - control = [approach+0Ch]; control and leader = [control+3D0h] non-null: the
//    leader's pose refreshed; v = (leader+ECh, 0.0, leader+F4h) (leader+F0h is read and
//    overwritten, 009BE951-009BE972); n = 00419510(scratch, v); end = state+30h + n x
//    100.0 (qword; 009BE9AD DC C9 = FMUL ST1,ST0, each product and sum stored as a
//    float); 0085F990(state+30h, end, green, green) at 009BEA93.
// x87 at 24-bit precision (each product and sum rounds once to float under either
// control word). Debug output only; no draw.
void bot_state_follow_debug_draw_009be590(Cc13Seg67nHost& host, void* state) {
    void* const approach = ptr_at(state, 0x4);
    if (at<std::uint8_t>(state, 0x85) == 0) {
        void* const unit = ptr_at(approach, 0x4);
        refresh_pose_if_stale(host, unit);
        host.line_rgba_floats_0085f990(static_cast<const float*>(ptr(unit, 0xFC)),
                                       static_cast<const float*>(ptr(state, 0x60)), kWhite, kGreen);
        if (at<float>(state, 0x5C) > 0.0f) {
            host.debug_circle64_00860710(static_cast<const float*>(ptr(state, 0x50)), kGreen,
                                         at<float>(state, 0x5C));
        }
    }
    const std::uint32_t game = host.global_dword_00e188a8();
    host.diagnostic_printf_004c7070_2d(game, kDebugTextX_00ce3930, kDebugTextY_00ce4bc4,
                                       reinterpret_cast<const char*>(
                                           static_cast<std::uintptr_t>(kStrFollowFormat_00d209d8)),
                                       static_cast<double>(at<float>(state, 0x90)),
                                       static_cast<double>(at<float>(state, 0x94)));
    const float* const point44 = static_cast<const float*>(ptr(state, 0x44));
    host.wire_sphere_00860be0(point44, kRed, kSphereRadius3_00ce3854, kRed, 1);
    {
        void* const unit = ptr_at(ptr_at(state, 0x4), 0x4);
        refresh_pose_if_stale(host, unit);
        host.line_rgba_floats_0085f990(static_cast<const float*>(ptr(unit, 0xFC)), point44, kWhite,
                                       kRed);
    }
    const float* const point30 = static_cast<const float*>(ptr(state, 0x30));
    host.wire_sphere_00860be0(point30, kGreen, kSphereRadius5_00ce3850, kGreen, 1);
    void* const control = ptr_at(ptr_at(state, 0x4), 0xC);
    if (control == nullptr || at<std::uint32_t>(control, 0x3D0) == 0) return;
    void* const leader = ptr_at(control, 0x3D0);
    refresh_pose_if_stale(host, leader);
    float v[3] = {at<float>(leader, 0xEC), at<float>(leader, 0xF0), at<float>(leader, 0xF4)};
    v[1] = 0.0f;
    float scratch[3] = {};
    const float* const n = host.vector3f_normalize_00419510(scratch, v);
    const float c[3] = {n[0], n[1], n[2]};
    const float d[3] = {static_cast<float>(static_cast<double>(c[0]) * kHundred_00d7a220),
                        static_cast<float>(static_cast<double>(c[1]) * kHundred_00d7a220),
                        static_cast<float>(static_cast<double>(c[2]) * kHundred_00d7a220)};
    const float end[3] = {point30[0] + d[0], point30[1] + d[1], point30[2] + d[2]};
    host.line_rgba_floats_0085f990(point30, end, kGreen, kGreen);
}

// 009DAA60: RET 8.
void ship_ai_state_debug_draw_noop_009daa60(void*, std::uint32_t, std::uint32_t) noexcept {}

// 009DAF60 (009DAF60-009DAFF3): the yellow static (00F8A6F0 = {1, 1, 0, 1}, guard
// 00F8A700); unit = [[state+4h]+AA8h], pose refreshed; 0085FFF0(unit+FCh, state+18h,
// yellow) at 009DAFEC. Both stack arguments unread; RET 8.
void ship_ai_land_debug_draw_009daf60(Cc13Seg67nHost& host, void* state) {
    void* const unit = ptr_at(ptr_at(state, 0x4), 0xAA8);
    refresh_pose_if_stale(host, unit);
    host.arrow_rgba_floats_one_colour_0085fff0(static_cast<const float*>(ptr(unit, 0xFC)),
                                               static_cast<const float*>(ptr(state, 0x18)), kYellow);
}

std::uint32_t ship_ai_stop_command_009dac80() noexcept { return 0x00E08F88u; }
std::uint32_t ship_ai_follow_command_009dadb0() noexcept { return 0x00E08F60u; }
std::uint32_t ship_ai_land_command_009daf50() noexcept { return 0x00E08FA0u; }
std::uint32_t ship_ai_moveonpath_command_009db050() noexcept { return 0x00E08F80u; }
std::uint32_t ship_ai_kamikaze_attack_command_009db350() noexcept { return 0x00E08F78u; }
std::uint32_t ship_ai_attack_substate_command_009db5a0() noexcept { return 0x00E08F78u; }
std::uint32_t ship_ai_attack_substate_command_009db610() noexcept { return 0x00E08F78u; }
std::uint32_t ship_ai_attack_substate_command_009db690() noexcept { return 0x00E08F78u; }
std::uint32_t ship_ai_attack_substate_command_009db7e0() noexcept { return 0x00E08F78u; }

}  // namespace bsp
